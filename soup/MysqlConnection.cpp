#include "MysqlConnection.hpp"

#if !SOUP_WASM

#include "Mixed.hpp"
#include "MysqlAuthSwitchRequest.hpp"
#include "MysqlCapabilities.hpp"
#include "MysqlFieldType.hpp"
#include "MysqlFrame.hpp"
#include "MysqlHandshake.hpp"
#include "MysqlHandshakeResponse.hpp"
#include "MysqlStmtExecute.hpp"
#include "MysqlStmtPrepareOk.hpp"
#include "sha1.hpp"
#include "string.hpp"

NAMESPACE_SOUP
{
	[[nodiscard]] static std::string mysql_native_auth(const std::string& password, std::string salt)
	{
		std::string password_sha1 = sha1::hash(password);
		std::string password_sha1_sha1 = sha1::hash(password_sha1);
		salt.append(password_sha1_sha1);
		salt = sha1::hash(std::move(salt));
		return string::xorSameLength(password_sha1, salt);
	}

	[[nodiscard]] static std::string mysqlHumaniseErrPacket(std::string&& data)
	{
		// error code
		data.erase(0, 2);

		// sql state
		data.append(" (");
		data.append(data.substr(0, 5));
		data.push_back(')');
		data.erase(0, 6);

		return data;
	}

	struct CaptureMysqlAuthenticate
	{
		std::string username;
		std::string password;
		MysqlConnection::auth_success_callback_t on_success;
		MysqlConnection::error_callback_t on_error;
	};

	void MysqlConnection::authenticate(std::string username, std::string password, auth_success_callback_t on_success, error_callback_t on_error)
	{
		mysqlRecv([](MysqlConnection& con, std::string&& data, Capture&& _cap)
		{
			auto& cap = _cap.get<CaptureMysqlAuthenticate>();

			if ((uint8_t)data.at(0) == 0xFF)
			{
				cap.on_error(con, data.substr(3));
				return;
			}

			MysqlHandshake hs;
			hs.fromBinaryLE(data);
			std::string salt = hs.salt_pt_1;
			salt.append(hs.salt_pt_2);
			//std::cout << "Version: " << hs.human_readable_version << std::endl;
			//std::cout << "Salt: " << salt << std::endl;
			//std::cout << "Auth method: " << hs.auth_plugin_name << std::endl;

			MysqlHandshakeResponse res{};
			res.client_capabilities = (CLIENT_PROTOCOL_41 | CLIENT_PROTOCOL_41_AUTH | CLIENT_PLUGIN_AUTH | CLIENT_DEPRECATE_EOF);
			res.max_packet = 0xFFFFFE;
			res.charset = 0xFF;
			res.username = std::move(cap.username);
			res.password = mysql_native_auth(cap.password, salt);
			res.client_auth_plugin = "mysql_native_password";
			con.mysqlSend(res);

			con.mysqlRecv([](MysqlConnection& con, std::string&& data, Capture&& _cap)
			{
				auto& cap = _cap.get<CaptureMysqlAuthenticate>();

				auto status = (uint8_t)data.at(0);
				data.erase(0, 1);
				//std::cout << "Status: " << (int)status << std::endl;
				if (status == 0)
				{
					cap.on_success(con);
				}
				else if (status == 0xFE)
				{
					MysqlAuthSwitchRequest req{};
					req.fromBinaryLE(data);

					//std::cout << "Salt: " << req.plugin_data << std::endl;
					con.mysqlSend(mysql_native_auth(cap.password, req.plugin_data));

					con.mysqlRecv([](MysqlConnection& sock, std::string&& data, Capture&& _cap)
					{
						auto& cap = _cap.get<CaptureMysqlAuthenticate>();

						auto status = (uint8_t)data.at(0);
						data.erase(0, 1);
						
						if (status == 0)
						{
							cap.on_success(static_cast<MysqlConnection&>(sock));
						}
						else //if (status == 0xFF)
						{
							cap.on_error(static_cast<MysqlConnection&>(sock), mysqlHumaniseErrPacket(std::move(data)));
						}
					});
				}
				else //if (status == 0xFF)
				{
					cap.on_error(static_cast<MysqlConnection&>(con), mysqlHumaniseErrPacket(std::move(data)));
				}
			}, std::move(_cap));
		}, CaptureMysqlAuthenticate{
			std::move(username),
			std::move(password),
			on_success,
			on_error
		});
	}

	struct CaptureQuery
	{
		MysqlConnection::query_success_callback_t on_success;
		MysqlConnection::error_callback_t on_error;
	};

	struct CaptureQueryRecvResponse
	{
		MysqlConnection::query_success_callback_t on_success;
		uint8_t num_results;
		uint8_t remaining_column_definitions;
		bool will_have_rows;
		bool will_have_eof;
		std::vector<std::pair<MysqlColumnDefinition, std::string>> result{};
	};

	void MysqlConnection::query(std::string statement, query_success_callback_t on_success, error_callback_t on_error)
	{
		statement.insert(0, 1, '\x3');
		next_send_seq_id = 0;
		mysqlSend(statement);

		mysqlRecv([](MysqlConnection& con, std::string&& data, Capture&& _cap)
		{
			auto& cap = _cap.get<CaptureQuery>();

			auto num_columns = (uint8_t)data.at(0);
			if (num_columns == 0xFF)
			{
				cap.on_error(con, mysqlHumaniseErrPacket(data.substr(1)));
				return;
			}

			bool will_have_data = (num_columns != 0);
			con.queryRecvResponse(CaptureQueryRecvResponse{
				cap.on_success,
				num_columns,
				num_columns,
				will_have_data,
				will_have_data
			});
		}, CaptureQuery{
			on_success,
			on_error
		});
	}

	void MysqlConnection::queryRecvResponse(Capture&& _cap)
	{
		auto& cap = _cap.get<CaptureQueryRecvResponse>();
		if (cap.remaining_column_definitions == 0)
		{
			if (cap.will_have_rows)
			{
				cap.will_have_rows = false;
				mysqlRecv([](MysqlConnection& con, std::string&& data, Capture&& _cap)
				{
					auto& cap = _cap.get<CaptureQueryRecvResponse>();

					StringReader r(std::move(data), true);
					for (uint8_t i = 0; i != cap.num_results; ++i)
					{
						r.str_lp_mysql(cap.result.at(i).second);
					}

					con.queryRecvResponse(std::move(_cap));
				}, std::move(_cap));
			}
			else if (cap.will_have_eof)
			{
				cap.will_have_eof = false;
				mysqlRecv([](MysqlConnection& con, std::string&& data, Capture&& cap)
				{
					con.queryRecvResponse(std::move(cap));
				}, std::move(_cap));
			}
			else
			{
				cap.on_success(*this, std::move(cap.result));
			}
		}
		else
		{
			--cap.remaining_column_definitions;
			mysqlRecv([](MysqlConnection& con, std::string&& data, Capture&& cap)
			{
				MysqlColumnDefinition col;
				col.fromBinaryLE(std::move(data));

				cap.get<CaptureQueryRecvResponse>().result.emplace_back(std::move(col), std::string());

				con.queryRecvResponse(std::move(cap));
			}, std::move(_cap));
		}
	}

	struct CaptureQueryWithArgs
	{
		std::vector<Mixed> args;
		MysqlConnection::query_with_args_success_callback_t on_success;
		MysqlConnection::error_callback_t on_error;
	};

	struct CaptureQueryWithArgsExecute
	{
		MysqlConnection::query_with_args_success_callback_t on_success;
		uint32_t stmt_id;
	};

	void MysqlConnection::query(std::string statement, std::vector<Mixed>&& args, query_with_args_success_callback_t on_success, error_callback_t on_error)
	{
		stmtPrepare(std::move(statement), [](MysqlConnection& con, uint32_t stmt_id, Capture&& _cap)
		{
			auto& cap = _cap.get<CaptureQueryWithArgs>();
			con.stmtExecute(stmt_id, cap.args, [](MysqlConnection& con, std::vector<std::pair<MysqlColumnDefinition, Mixed>>&& result, Capture&& _cap)
			{
				auto& cap = _cap.get<CaptureQueryWithArgsExecute>();

				con.stmtClose(cap.stmt_id);

				cap.on_success(con, std::move(result));
			}, cap.on_error, CaptureQueryWithArgsExecute{
				cap.on_success,
				stmt_id
			});
		}, CaptureQueryWithArgs{
			std::move(args),
			on_success,
			on_error
		});
	}

	struct CaptureStmtPrepare
	{
		MysqlConnection::stmt_prepare_callback_t callback;
		Capture cb_cap;
	};

	struct CaptureStmtPrepareRecvResponse : public CaptureStmtPrepare
	{
		uint32_t stmt_id;
		uint32_t num_packets_nobody_needs_remaining;
	};

	void MysqlConnection::stmtPrepare(std::string statement, stmt_prepare_callback_t callback, Capture&& cb_cap)
	{
		statement.insert(0, 1, '\x16');
		next_send_seq_id = 0;
		mysqlSend(statement);

		mysqlRecv([](MysqlConnection& con, std::string&& data, Capture&& _cap)
		{
			auto& cap = _cap.get<CaptureStmtPrepare>();

			if (data.at(0) != 0)
			{
				// I see no good reason for this to ever have an error
				cap.callback(con, -2, std::move(cap.cb_cap));
				return;
			}
			data.erase(0, 1);
			MysqlStmtPrepareOk ok;
			ok.fromBinaryLE(std::move(data));
			con.stmtPrepareRecvResponse(CaptureStmtPrepareRecvResponse{
				std::move(cap),
				ok.stmt_id,
				(uint32_t)(ok.num_params + ok.num_columns)
			});
		}, CaptureStmtPrepare{
			callback,
			std::move(cb_cap)
		});
	}

	void MysqlConnection::stmtPrepareRecvResponse(Capture&& _cap)
	{
		auto& cap = _cap.get<CaptureStmtPrepareRecvResponse>();
		if (cap.num_packets_nobody_needs_remaining == 0)
		{
			cap.callback(*this, cap.stmt_id, std::move(cap.cb_cap));
			return;
		}
		--cap.num_packets_nobody_needs_remaining;
		mysqlRecv([](MysqlConnection& con, std::string&& data, Capture&& cap)
		{
			con.stmtPrepareRecvResponse(std::move(cap));
		}, std::move(_cap));
	}

	struct CaptureStmtExecute
	{
		MysqlConnection::stmt_execute_success_callback_t on_success;
		MysqlConnection::error_callback_t on_error;
		Capture success_cap;
	};

	struct CaptureStmtExecuteRecvResponse : public CaptureStmtExecute
	{
		uint8_t num_results;
		uint8_t remaining_column_definitions;
		bool will_have_rows;
		bool will_have_eof;
		std::vector<std::pair<MysqlColumnDefinition, Mixed>> result{};
	};

	void MysqlConnection::stmtExecute(uint32_t stmt_id, const std::vector<Mixed>& args, stmt_execute_success_callback_t on_success, error_callback_t on_error, Capture&& success_cap)
	{
		StringWriter w(true);
		uint8_t header = 0x17;
		w.u8(header);

		MysqlStmtExecute exec;
		exec.stmt_id = stmt_id;
		exec.flags = 0;
		exec.iterations = 1;
		exec.write(w);
		
		uint8_t null_bitmap = 0;
		for (auto i = ((args.size() + 7) / 8); i-- != 0; )
		{
			w.u8(null_bitmap);
		}

		uint8_t send_type_to_server = 1;
		w.u8(send_type_to_server);

		for (const auto& arg : args)
		{
			uint8_t field_type = (arg.isString() ? MYSQL_TYPE_STRING : MYSQL_TYPE_LONGLONG);
			w.u8(field_type);

			uint8_t param_flag = (arg.isUInt() ? 128 : 0);
			w.u8(param_flag);
		}

		for (const auto& arg : args)
		{
			if (arg.isInt())
			{
				auto val = arg.getInt();
				w.i64le(val);
			}
			else if (arg.isUInt())
			{
				auto val = arg.getUInt();
				w.u64le(val);
			}
			else
			{
				w.str_lp_mysql(arg.toString());
			}
		}

		next_send_seq_id = 0;
		mysqlSend(w.data);

		mysqlRecv([](MysqlConnection& con, std::string&& data, Capture&& _cap)
		{
			auto& cap = _cap.get<CaptureStmtExecute>();

			auto num_columns = (uint8_t)data.at(0);
			if (num_columns == 0xFF)
			{
				cap.on_error(con, mysqlHumaniseErrPacket(data.substr(1)));
				return;
			}

			bool will_have_data = (num_columns != 0);
			con.stmtExecuteRecvResponse(CaptureStmtExecuteRecvResponse{
				std::move(cap),
				num_columns,
				num_columns,
				will_have_data,
				will_have_data
			});
		}, CaptureStmtExecute{
			on_success,
			on_error,
			std::move(success_cap)
		});
	}

	void MysqlConnection::stmtExecuteRecvResponse(Capture&& _cap)
	{
		auto& cap = _cap.get<CaptureStmtExecuteRecvResponse>();
		if (cap.remaining_column_definitions == 0)
		{
			if (cap.will_have_rows)
			{
				cap.will_have_rows = false;
				mysqlRecv([](MysqlConnection& con, std::string&& data, Capture&& _cap)
				{
					auto& cap = _cap.get<CaptureStmtExecuteRecvResponse>();

					StringReader r(std::move(data), true);
					
					uint8_t header;
					r.u8(header); // should be 0

					uint8_t null_bitmap;
					for (auto i = ((cap.num_results + 7) / 8); i-- != 0; )
					{
						r.u8(null_bitmap);
					}

					for (uint8_t i = 0; i != cap.num_results; ++i)
					{
						switch (cap.result.at(i).first.type)
						{
						case MYSQL_TYPE_LONGLONG:
							{
								if (cap.result.at(i).first.flags & 32)
								{
									uint64_t val;
									r.u64be(val);
									cap.result.at(i).second = val;
								}
								else
								{
									int64_t val;
									r.i64be(val);
									cap.result.at(i).second = val;
								}
							}
							break;

						case MYSQL_TYPE_STRING:
						case MYSQL_TYPE_VARCHAR:
						case MYSQL_TYPE_VAR_STRING:
						case MYSQL_TYPE_ENUM:
						case MYSQL_TYPE_SET:
						case MYSQL_TYPE_LONG_BLOB:
						case MYSQL_TYPE_MEDIUM_BLOB:
						case MYSQL_TYPE_BLOB:
						case MYSQL_TYPE_TINY_BLOB:
						case MYSQL_TYPE_GEOMETRY:
						case MYSQL_TYPE_BIT:
						case MYSQL_TYPE_DECIMAL:
						case MYSQL_TYPE_NEWDECIMAL:
							{
								std::string val{};
								r.str_lp_mysql(val);
								cap.result.at(i).second = std::move(val);
								break;
							}

						default:
							{
								std::string msg = "[soup] Unsupported response type: ";
								msg.append(std::to_string(cap.result.at(i).first.type));
								cap.on_error(con, std::move(msg));
								return;
							}
						}
					}

					con.stmtExecuteRecvResponse(std::move(_cap));
				}, std::move(_cap));
			}
			else if (cap.will_have_eof)
			{
				cap.will_have_eof = false;
				mysqlRecv([](MysqlConnection& con, std::string&& data, Capture&& cap)
				{
					con.stmtExecuteRecvResponse(std::move(cap));
				}, std::move(_cap));
			}
			else
			{
				cap.on_success(*this, std::move(cap.result), std::move(cap.success_cap));
			}
		}
		else
		{
			--cap.remaining_column_definitions;
			mysqlRecv([](MysqlConnection& con, std::string&& data, Capture&& cap)
			{
				MysqlColumnDefinition col;
				col.fromBinaryLE(std::move(data));

				cap.get<CaptureStmtExecuteRecvResponse>().result.emplace_back(std::move(col), Mixed());

				con.stmtExecuteRecvResponse(std::move(cap));
			}, std::move(_cap));
		}
	}

	struct CaptureMysqlRecv
	{
		MysqlConnection::recv_callback_t callback;
		Capture cap;
	};

	void MysqlConnection::stmtClose(uint32_t stmt_id)
	{
		StringWriter w(true);
		uint8_t header = 0x19;
		w.u8(header);

		w.u32be(stmt_id);

		mysqlSend(std::move(w.data));
	}

	void MysqlConnection::mysqlRecv(recv_callback_t callback, Capture&& cap)
	{
		if (recv_buf.size() >= 4)
		{
			MysqlFrame frame;
			frame.fromBinaryLE(recv_buf.substr(0, 4));

			if (recv_buf.size() >= frame.length + 4)
			{
				next_send_seq_id = (frame.seq_id + 1);

				std::string data = recv_buf.substr(4, frame.length);
				recv_buf.erase(0, frame.length + 4);

				callback(*this, std::move(data), std::move(cap));
				return;
			}
		}

		recv([](Socket& sock, std::string&& data, Capture&& _cap) SOUP_EXCAL
		{
			auto& con = static_cast<MysqlConnection&>(sock);
			auto& cap = _cap.get<CaptureMysqlRecv>();

			con.recv_buf.append(data);

			con.mysqlRecv(cap.callback, std::move(cap.cap));
		}, CaptureMysqlRecv{
			callback,
			std::move(cap)
		});
	}

	void MysqlConnection::mysqlSend(std::string data)
	{
		MysqlFrame frame{};
		frame.length = static_cast<decltype(frame.length)>(data.size());
		frame.seq_id = next_send_seq_id++;
		data.insert(0, frame.toBinaryStringLE());
		send(data);
	}
}

#endif

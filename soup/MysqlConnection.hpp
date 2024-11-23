#pragma once

#include "Socket.hpp"
#if !SOUP_WASM

#include "Callback.hpp"
#include "MysqlColumnDefinition.hpp"

NAMESPACE_SOUP
{
	class MysqlConnection : public Socket
	{
	private:
		std::string recv_buf{};
		uint8_t next_send_seq_id = 0;

	public:
		using auth_success_callback_t = void(*)(MysqlConnection&);
		using error_callback_t = void(*)(MysqlConnection&, std::string&& human_readable_error_message);
		using query_success_callback_t = void(*)(MysqlConnection&, std::vector<std::pair<MysqlColumnDefinition, std::string>>&&);
		using query_with_args_success_callback_t = void(*)(MysqlConnection&, std::vector<std::pair<MysqlColumnDefinition, Mixed>>&&);
		using recv_callback_t = void(*)(MysqlConnection&, std::string&&, Capture&&);
		using stmt_prepare_callback_t = void(*)(MysqlConnection&, uint32_t stmt_id, Capture&&);
		using stmt_execute_success_callback_t = void(*)(MysqlConnection&, std::vector<std::pair<MysqlColumnDefinition, Mixed>>&&, Capture&&);

		// Call after connected. Will use mysql_native_password.
		void authenticate(std::string username, std::string password, auth_success_callback_t on_success, error_callback_t on_error);

		void query(std::string statement, query_success_callback_t on_success, error_callback_t on_error);
	private:
		void queryRecvResponse(Capture&& _cap);

	public:
		void query(std::string statement, std::vector<Mixed>&& args, query_with_args_success_callback_t on_success, error_callback_t on_error);

		void stmtPrepare(std::string statement, stmt_prepare_callback_t callback, Capture&& cb_cap = {});
	private:
		void stmtPrepareRecvResponse(Capture&& _cap);

	public:
		void stmtExecute(uint32_t stmt_id, const std::vector<Mixed>& args, stmt_execute_success_callback_t on_success, error_callback_t on_error, Capture&& success_cap = {});
	private:
		void stmtExecuteRecvResponse(Capture&& _cap);

	public:
		void stmtClose(uint32_t stmt_id);

		void mysqlRecv(recv_callback_t callback, Capture&& cap = {});

		template <typename T>
		void mysqlSend(T& packet)
		{
			return mysqlSend(packet.toBinaryString());
		}

		void mysqlSend(std::string data);
	};
}
#endif

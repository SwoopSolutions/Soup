#include "laWasmBackend.hpp"

#include "StringWriter.hpp"
#include "utility.hpp" // SOUP_MOVE_RETURN

NAMESPACE_SOUP
{
	void laWasmBackend::linkPosix(irModule& m)
	{
		for (size_t i = 0; i != m.imports.size(); ++i)
		{
			if (m.imports[i].module_name == "posix")
			{
				if (m.imports[i].func.name == "write")
				{
					m.updateCalls(~i, m.func_exports.size());

					m.imports[i].module_name = "wasi_snapshot_preview1";
					m.imports[i].func.name = "fd_write";
					m.imports[i].func.parameters = { IR_I32, IR_PTR, IR_I32, IR_PTR };
					m.imports[i].func.returns = { IR_I32 };

					const auto ptr = m.allocateZeroedMemory(8, 4);

					irFunction& fn = m.func_exports.emplace_back();
					fn.name = "posix_write";
					fn.parameters = { IR_I32, IR_PTR, IR_I64 };
					fn.returns.emplace_back(IR_I64);
					{
						auto storeInsn = soup::make_unique<irExpression>(IR_STORE_I32);
						{
							auto ptrConst = soup::make_unique<irExpression>(IR_CONST_PTR);
							ptrConst->const_ptr.value = ptr + 0;
							storeInsn->children.emplace_back(std::move(ptrConst));
						}
						{
							auto localGetExpr = soup::make_unique<irExpression>(IR_LOCAL_GET);
							localGetExpr->local_get.index = 1;
							storeInsn->children.emplace_back(std::move(localGetExpr));
						}
						fn.insns.emplace_back(std::move(storeInsn));
					}
					{
						auto storeInsn = soup::make_unique<irExpression>(IR_STORE_I32);
						{
							auto ptrConst = soup::make_unique<irExpression>(IR_CONST_PTR);
							ptrConst->const_ptr.value = ptr + 4;
							storeInsn->children.emplace_back(std::move(ptrConst));
						}
						{
							auto castInsn = soup::make_unique<irExpression>(IR_I64_TO_I32);
							{
								auto localGetExpr = soup::make_unique<irExpression>(IR_LOCAL_GET);
								localGetExpr->local_get.index = 2;
								castInsn->children.emplace_back(std::move(localGetExpr));
							}
							storeInsn->children.emplace_back(std::move(castInsn));
						}
						fn.insns.emplace_back(std::move(storeInsn));
					}
					{
						auto returnInsn = soup::make_unique<irExpression>(IR_RET);
						{
							auto castInsn = soup::make_unique<irExpression>(IR_I32_TO_I64_ZX);
							{
								auto callInsn = soup::make_unique<irExpression>(IR_CALL);
								callInsn->call.index = ~i;
								{
									auto localGetExpr = soup::make_unique<irExpression>(IR_LOCAL_GET);
									localGetExpr->local_get.index = 0;
									callInsn->children.emplace_back(std::move(localGetExpr));
								}
								{
									auto ptrConst = soup::make_unique<irExpression>(IR_CONST_PTR);
									ptrConst->const_ptr.value = ptr + 0;
									callInsn->children.emplace_back(std::move(ptrConst));
								}
								{
									auto constI32Expr = soup::make_unique<irExpression>(IR_CONST_I32);
									constI32Expr->const_i32.value = 1;
									callInsn->children.emplace_back(std::move(constI32Expr));
								}
								{
									auto ptrConst = soup::make_unique<irExpression>(IR_CONST_PTR);
									ptrConst->const_ptr.value = ptr + 0;
									callInsn->children.emplace_back(std::move(ptrConst));
								}
								castInsn->children.emplace_back(std::move(callInsn));
							}
							returnInsn->children.emplace_back(std::move(castInsn));
						}
						fn.insns.emplace_back(std::move(returnInsn));
					}
				}
			}
		}
	}

	std::string laWasmBackend::compileModule(const irModule& m) const
	{
		StringWriter w;

		uint8_t b; uint32_t u;
		u = 1836278016; w.u32le(u);
		u = 1; w.u32le(u);

		// Type section
		{
			b = 1; w.u8(b);
			auto data = getTypeSectionData(m);
			w.oml(data.size());
			w.raw(data.data(), data.size());
		}

		// Import section
		{
			b = 2; w.u8(b);
			auto data = getImportSectionData(m);
			w.oml(data.size());
			w.raw(data.data(), data.size());
		}

		// Function section
		{
			b = 3; w.u8(b);
			auto data = getFunctionSectionData(m);
			w.oml(data.size());
			w.raw(data.data(), data.size());
		}

		// Memory section
		{
			b = 5; w.u8(b);
			auto data = getMemorySectionData(m);
			w.oml(data.size());
			w.raw(data.data(), data.size());
		}

		// Export section
		{
			b = 7; w.u8(b);
			auto data = getExportSectionData(m);
			w.oml(data.size());
			w.raw(data.data(), data.size());
		}

		// Code section
		{
			b = 10; w.u8(b);
			auto data = getCodeSectionData(m);
			w.oml(data.size());
			w.raw(data.data(), data.size());
		}

		// Data section
		if (!m.data.empty())
		{
			b = 11; w.u8(b);
			auto data = getDataSectionData(m);
			w.oml(data.size());
			w.raw(data.data(), data.size());
		}

		SOUP_MOVE_RETURN(w.data);
	}

	std::string laWasmBackend::getTypeSectionData(const irModule& m)
	{
		StringWriter w;
		w.oml(m.imports.size() + m.func_exports.size());
		for (const auto& imp : m.imports)
		{
			uint8_t type_type = 0x60; w.u8(type_type); // function
			w.oml(imp.func.parameters.size());
			for (const auto& t : imp.func.parameters)
			{
				writeType(w, t);
			}
			w.oml(imp.func.returns.size());
			for (const auto& t : imp.func.returns)
			{
				writeType(w, t);
			}
		}
		for (const auto& func : m.func_exports)
		{
			uint8_t type_type = 0x60; w.u8(type_type); // function
			w.oml(func.parameters.size());
			for (const auto& t : func.parameters)
			{
				writeType(w, t);
			}
			w.oml(func.returns.size());
			for (const auto& t : func.returns)
			{
				writeType(w, t);
			}
		}
		SOUP_MOVE_RETURN(w.data);
	}

	std::string laWasmBackend::getImportSectionData(const irModule& m)
	{
		StringWriter w;
		w.oml(m.imports.size());
		for (uint32_t i = 0; i != m.imports.size(); ++i)
		{
			const auto& imp = m.imports[i];
			w.oml(imp.module_name.size());
			w.str(imp.module_name.size(), imp.module_name.data());
			w.oml(imp.func.name.size());
			w.str(imp.func.name.size(), imp.func.name.data());
			uint8_t kind = 0; w.u8(kind); // function
			w.oml(i); // type index
		}
		SOUP_MOVE_RETURN(w.data);
	}

	std::string laWasmBackend::getFunctionSectionData(const irModule& m)
	{
		StringWriter w;
		w.oml(m.func_exports.size());
		for (uint32_t i = 0; i != m.func_exports.size(); ++i)
		{
			w.oml(i + m.imports.size()); // We're keeping function & type indecies the same for now for simplicity.
		}
		SOUP_MOVE_RETURN(w.data);
	}

	std::string laWasmBackend::getMemorySectionData(const irModule& m)
	{
		StringWriter w;
		w.oml(1); // num memories
		w.skip(1); // flags
		w.oml(1); // num pages
		SOUP_MOVE_RETURN(w.data);
	}

	std::string laWasmBackend::getExportSectionData(const irModule& m)
	{
		StringWriter w;
		w.oml(m.func_exports.size() + 1);
		for (uint32_t i = 0; i != m.func_exports.size(); ++i)
		{
			const auto& func = m.func_exports[i];
			w.oml(func.name.size());
			w.str(func.name.size(), func.name.data());
			uint8_t kind = 0; w.u8(kind); // function
			w.oml(m.imports.size() + i);
		}
		{
			w.oml(6);
			w.str(6, "memory");
			uint8_t kind = 2; w.u8(kind); // memory
			w.oml(0);
		}
		SOUP_MOVE_RETURN(w.data);
	}

	std::string laWasmBackend::getCodeSectionData(const irModule& m)
	{
		StringWriter w;
		w.oml(m.func_exports.size());
		for (const auto& func : m.func_exports)
		{
			auto body = compileFunction(m, func);
			w.oml(body.size());
			w.str(body.size(), body.data());
		}
		SOUP_MOVE_RETURN(w.data);
	}

	std::string laWasmBackend::getDataSectionData(const irModule& m)
	{
		StringWriter w;
		uint8_t b;
		w.oml(1); // num segments
		w.skip(1); // flags
		b = 0x41; w.u8(b); // i32.const
		w.soml(0); // base
		b = 0x0b; w.u8(b); // end
		auto data = m.getContiguousMemory();
		w.oml(data.size());
		w.str(data.size(), data.data());
		SOUP_MOVE_RETURN(w.data);
	}

	bool laWasmBackend::writeType(StringWriter& w, irType type)
	{
		uint8_t b;
		switch (type)
		{
		case IR_BOOL: b = 0x7f; break; // i32
		case IR_I8: b = 0x7f; break; // i32
		case IR_I32: b = 0x7f; break; // i32
		case IR_I64: b = 0x7e; break; // i64
		case IR_PTR: b = 0x7f; break; // i32
		}
		return w.u8(b);
	}

	std::string laWasmBackend::compileFunction(const irModule& m, const irFunction& fn)
	{
		StringWriter w;
		w.oml(fn.locals.size());
		for (const auto& local : fn.locals)
		{
			w.oml(1);
			writeType(w, local);
		}
		auto i = fn.insns.begin();
		if (true) // Optimisation: Skip zero-initialisation of locals, because WASM guarantees it.
		{
			for (; i != fn.insns.end(); ++i)
			{
				irExpression& insn = **i;
				if (insn.type != IR_LOCAL_SET
					|| !insn.children.at(0)->isConstantZero()
					)
				{
					break;
				}
			}
		}
		for (; i != fn.insns.end(); ++i)
		{
			irExpression& insn = **i;
			int ret = compileExpression(m, w, insn);
			if (insn.type == IR_RET)
			{
				break;
			}
			discard(w, ret);
		}
		uint8_t b = 0x0b; w.u8(b); // end
		SOUP_MOVE_RETURN(w.data);
	}

	int laWasmBackend::compileExpression(const irModule& m, StringWriter& w, const irExpression& e)
	{
		uint8_t b;
		switch (e.type)
		{
		case IR_CONST_BOOL:
			b = 0x41; w.u8(b); // i32.const
			w.soml(e.const_bool.value ? 1 : 0);
			return 1;

		case IR_CONST_I8:
			b = 0x41; w.u8(b); // i32.const
			w.soml(e.const_i8.value);
			return 1;

		case IR_CONST_I32:
			b = 0x41; w.u8(b); // i32.const
			w.soml(e.const_i32.value);
			return 1;

		case IR_CONST_I64:
			b = 0x42; w.u8(b); // i64.const
			w.soml(e.const_i64.value);
			return 1;

		case IR_CONST_PTR:
			b = 0x41; w.u8(b); // i32.const
			w.soml(e.const_ptr.value);
			return 1;

		case IR_LOCAL_GET:
			b = 0x20; w.u8(b); // local.get
			w.oml(e.local_get.index);
			return 1;

		case IR_LOCAL_SET:
			compileExpression(m, w, *e.children.at(0));
			b = 0x21; w.u8(b); // local.set
			w.oml(e.local_set.index);
			return 0;

		case IR_CALL:
			{
				int nargs = 0;
				for (const auto& child : e.children)
				{
					nargs += compileExpression(m, w, *child);
				}
				if (e.call.index >= 0)
				{
					auto overflow = static_cast<int>(nargs - m.func_exports.at(e.call.index).parameters.size());
					discard(w, overflow);
					b = 0x10; w.u8(b); // call
					w.oml(e.call.index + m.imports.size());
					return static_cast<int>(m.func_exports.at(e.call.index).returns.size());
				}
				else
				{
					auto overflow = static_cast<int>(nargs - m.imports.at(~e.call.index).func.parameters.size());
					discard(w, overflow);
					b = 0x10; w.u8(b); // call
					w.oml(~e.call.index);
					return static_cast<int>(m.imports.at(~e.call.index).func.returns.size());
				}
			}

		case IR_RET:
			{
				int ret = 0;
				for (const auto& child : e.children)
				{
					ret += compileExpression(m, w, *child);
				}
				return ret;
			}

		case IR_IFELSE:
			if (1 + e.ifelse.ifinsns == e.children.size()) // If without else?
			{
				SOUP_ASSERT(compileExpression(m, w, *e.children.at(0)) == 1);
				b = 0x04; w.u8(b); // if
				b = 0x40; w.u8(b); // void
				for (size_t i = 0; i != e.ifelse.ifinsns; ++i)
				{
					discard(w, compileExpression(m, w, *e.children[1 + i]));
				}
				b = 0x0b; w.u8(b); // end
			}
			else if (e.children.at(0)->isNegativeCompareToConstantZero())
			{
				auto inv_cond = e.children.at(0)->inverted();
				SOUP_ASSERT(compileExpression(m, w, *inv_cond) == 1);
				b = 0x04; w.u8(b); // if
				b = 0x40; w.u8(b); // void
				for (size_t i = 1 + e.ifelse.ifinsns; i != e.children.size(); ++i)
				{
					discard(w, compileExpression(m, w, *e.children[i]));
				}
				b = 0x05; w.u8(b); // else
				for (size_t i = 0; i != e.ifelse.ifinsns; ++i)
				{
					discard(w, compileExpression(m, w, *e.children[1 + i]));
				}
				b = 0x0b; w.u8(b); // end
			}
			else
			{
				SOUP_ASSERT(compileExpression(m, w, *e.children.at(0)) == 1);
				b = 0x04; w.u8(b); // if
				b = 0x40; w.u8(b); // void
				for (size_t i = 0; i != e.ifelse.ifinsns; ++i)
				{
					discard(w, compileExpression(m, w, *e.children[1 + i]));
				}
				b = 0x05; w.u8(b); // else
				for (size_t i = 1 + e.ifelse.ifinsns; i != e.children.size(); ++i)
				{
					discard(w, compileExpression(m, w, *e.children[i]));
				}
				b = 0x0b; w.u8(b); // end
			}
			return 0;

		case IR_WHILE:
			b = 0x03; w.u8(b); // loop
			b = 0x40; w.u8(b); // void
			if (false) // there may be some situations where inverting the condition could lead to faster code, but I'm not sure.
			{
				auto inv_cond = e.children.at(0)->inverted();
				b = 0x02; w.u8(b); // block
				b = 0x40; w.u8(b); // void
				SOUP_ASSERT(compileExpression(m, w, *inv_cond) == 1);
				b = 0x0d; w.u8(b); // br_if
				b = 0x00; w.u8(b); // to end of 'block'
				for (size_t i = 1; i != e.children.size(); ++i)
				{
					discard(w, compileExpression(m, w, *e.children[i]));
				}
				b = 0x0c; w.u8(b); // br
				b = 0x01; w.u8(b); // skip over 'block'; back to 'loop'
				b = 0x0b; w.u8(b); // end of 'block'
			}
			else
			{
				SOUP_ASSERT(compileExpression(m, w, *e.children.at(0)) == 1);
				b = 0x04; w.u8(b); // if
				b = 0x40; w.u8(b); // void
				for (size_t i = 1; i != e.children.size(); ++i)
				{
					discard(w, compileExpression(m, w, *e.children[i]));
				}
				b = 0x0c; w.u8(b); // br
				b = 0x01; w.u8(b); // skip over 'if'; back to 'loop'
				b = 0x0b; w.u8(b); // end of 'if'
			}
			b = 0x0b; w.u8(b); // end of 'loop'
			return 0;

		case IR_DISCARD:
			{
				auto nitems = compileExpression(m, w, *e.children.at(0));
				discard(w, static_cast<int>(e.discard.count));
				return static_cast<int>(nitems - e.discard.count);
			}

		case IR_ADD_I32:
		case IR_ADD_PTR:
			SOUP_ASSERT(e.children.size() == 2);
			SOUP_ASSERT(compileExpression(m, w, *e.children[0]) == 1);
			SOUP_ASSERT(compileExpression(m, w, *e.children[1]) == 1);
			b = 0x6a; w.u8(b); // i32.add
			return 1;

		case IR_ADD_I64:
			SOUP_ASSERT(e.children.size() == 2);
			SOUP_ASSERT(compileExpression(m, w, *e.children[0]) == 1);
			SOUP_ASSERT(compileExpression(m, w, *e.children[1]) == 1);
			b = 0x7c; w.u8(b); // i64.add
			return 1;

		case IR_SUB_I32:
			SOUP_ASSERT(e.children.size() == 2);
			SOUP_ASSERT(compileExpression(m, w, *e.children[0]) == 1);
			SOUP_ASSERT(compileExpression(m, w, *e.children[1]) == 1);
			b = 0x6b; w.u8(b); // i32.sub
			return 1;

		case IR_SUB_I64:
			SOUP_ASSERT(e.children.size() == 2);
			SOUP_ASSERT(compileExpression(m, w, *e.children[0]) == 1);
			SOUP_ASSERT(compileExpression(m, w, *e.children[1]) == 1);
			b = 0x7d; w.u8(b); // i64.sub
			return 1;

		case IR_MUL_I64:
			SOUP_ASSERT(e.children.size() == 2);
			SOUP_ASSERT(compileExpression(m, w, *e.children[0]) == 1);
			SOUP_ASSERT(compileExpression(m, w, *e.children[1]) == 1);
			b = 0x7e; w.u8(b); // i64.mul
			return 1;

		case IR_SDIV_I64:
			SOUP_ASSERT(e.children.size() == 2);
			SOUP_ASSERT(compileExpression(m, w, *e.children[0]) == 1);
			SOUP_ASSERT(compileExpression(m, w, *e.children[1]) == 1);
			b = 0x7f; w.u8(b); // i64.div_s
			return 1;

		case IR_UDIV_I64:
			SOUP_ASSERT(e.children.size() == 2);
			SOUP_ASSERT(compileExpression(m, w, *e.children[0]) == 1);
			SOUP_ASSERT(compileExpression(m, w, *e.children[1]) == 1);
			b = 0x80; w.u8(b); // i64.div_u
			return 1;

		case IR_SMOD_I64:
			SOUP_ASSERT(e.children.size() == 2);
			SOUP_ASSERT(compileExpression(m, w, *e.children[0]) == 1);
			SOUP_ASSERT(compileExpression(m, w, *e.children[1]) == 1);
			b = 0x81; w.u8(b); // i64.rem_s
			return 1;

		case IR_UMOD_I64:
			SOUP_ASSERT(e.children.size() == 2);
			SOUP_ASSERT(compileExpression(m, w, *e.children[0]) == 1);
			SOUP_ASSERT(compileExpression(m, w, *e.children[1]) == 1);
			b = 0x82; w.u8(b); // i64.rem_u
			return 1;


		case IR_EQUALS_I8:
		case IR_EQUALS_I32:
			SOUP_ASSERT(e.children.size() == 2);
			if (e.children[1]->isConstantZero())
			{
				SOUP_ASSERT(compileExpression(m, w, *e.children[0]) == 1);
				b = 0x45; w.u8(b); // i32.eqz
			}
			else if (e.children[0]->isConstantZero())
			{
				SOUP_ASSERT(compileExpression(m, w, *e.children[1]) == 1);
				b = 0x45; w.u8(b); // i32.eqz
			}
			else
			{
				SOUP_ASSERT(compileExpression(m, w, *e.children[0]) == 1);
				SOUP_ASSERT(compileExpression(m, w, *e.children[1]) == 1);
				b = 0x46; w.u8(b); // i32.eq
			}
			return 1;

		case IR_EQUALS_I64:
			SOUP_ASSERT(e.children.size() == 2);
			if (e.children[1]->isConstantZero())
			{
				SOUP_ASSERT(compileExpression(m, w, *e.children[0]) == 1);
				b = 0x50; w.u8(b); // i64.eqz
			}
			else if (e.children[0]->isConstantZero())
			{
				SOUP_ASSERT(compileExpression(m, w, *e.children[1]) == 1);
				b = 0x50; w.u8(b); // i64.eqz
			}
			else
			{
				SOUP_ASSERT(compileExpression(m, w, *e.children[0]) == 1);
				SOUP_ASSERT(compileExpression(m, w, *e.children[1]) == 1);
				b = 0x51; w.u8(b); // i64.eq
			}
			return 1;

		case IR_NOTEQUALS_I8:
		case IR_NOTEQUALS_I32:
			SOUP_ASSERT(e.children.size() == 2);
			SOUP_ASSERT(compileExpression(m, w, *e.children[0]) == 1);
			SOUP_ASSERT(compileExpression(m, w, *e.children[1]) == 1);
			b = 0x47; w.u8(b); // i32.ne
			return 1;

		case IR_NOTEQUALS_I64:
			SOUP_ASSERT(e.children.size() == 2);
			SOUP_ASSERT(compileExpression(m, w, *e.children[0]) == 1);
			SOUP_ASSERT(compileExpression(m, w, *e.children[1]) == 1);
			b = 0x52; w.u8(b); // i64.ne
			return 1;

		case IR_LOAD_I8:
			SOUP_ASSERT(compileExpression(m, w, *e.children.at(0)) == 1);
			b = 0x2d; w.u8(b); // i32.load8_u
			w.skip(2); // memflags + offset
			return 1;

		case IR_STORE_I8:
			SOUP_ASSERT(e.children.size() == 2);
			SOUP_ASSERT(compileExpression(m, w, *e.children[0]) == 1);
			SOUP_ASSERT(compileExpression(m, w, *e.children[1]) == 1);
			b = 0x3a; w.u8(b); // i32.store8
			w.skip(2); // memflags + offset
			return 0;

		case IR_STORE_I32:
			SOUP_ASSERT(e.children.size() == 2);
			SOUP_ASSERT(compileExpression(m, w, *e.children[0]) == 1);
			SOUP_ASSERT(compileExpression(m, w, *e.children[1]) == 1);
			b = 0x36; w.u8(b); // i32.store
			w.skip(2); // memflags + offset
			return 0;

		case IR_STORE_I64:
			SOUP_ASSERT(e.children.size() == 2);
			SOUP_ASSERT(compileExpression(m, w, *e.children[0]) == 1);
			SOUP_ASSERT(compileExpression(m, w, *e.children[1]) == 1);
			b = 0x37; w.u8(b); // i64.store
			w.skip(2); // memflags + offset
			return 0;

		case IR_I64_TO_PTR:
			SOUP_ASSERT(compileExpression(m, w, *e.children.at(0)) == 1);
			b = 0xa7; w.u8(b); // i32.wrap_i64
			return 1;

		case IR_I64_TO_I32:
			SOUP_ASSERT(compileExpression(m, w, *e.children.at(0)) == 1);
			b = 0xa7; w.u8(b); // i32.wrap_i64
			return 1;

		case IR_I64_TO_I8:
			SOUP_ASSERT(compileExpression(m, w, *e.children.at(0)) == 1);
			b = 0xa7; w.u8(b); // i32.wrap_i64
			// TODO: Modulo 0xff to ensure it's a valid i8 now
			return 1;

		case IR_I32_TO_I64_SX:
			SOUP_ASSERT(compileExpression(m, w, *e.children.at(0)) == 1);
			b = 0xac; w.u8(b); // i64.extend_i32_s
			return 1;

		case IR_I32_TO_I64_ZX:
			SOUP_ASSERT(compileExpression(m, w, *e.children.at(0)) == 1);
			b = 0xad; w.u8(b); // i64.extend_i32_u
			return 1;

		case IR_I8_TO_I64_SX:
			if (e.children.at(0)->type == IR_LOAD_I8)
			{
				SOUP_ASSERT(compileExpression(m, w, *e.children.at(0)->children.at(0)) == 1);
				b = 0x30; w.u8(b); // i64.load8_s
				w.skip(2); // memflags + offset
			}
			else
			{
				SOUP_ASSERT(compileExpression(m, w, *e.children.at(0)) == 1);
				/* First, sign-extend i8 to i32: */ b = 0xc0; w.u8(b); // i32.extend8_s
				/* Then, sign-extend i32 to i64: */ b = 0xac; w.u8(b); // i64.extend_i32_s
			}
			return 1;

		case IR_I8_TO_I64_ZX:
			if (e.children.at(0)->type == IR_LOAD_I8)
			{
				SOUP_ASSERT(compileExpression(m, w, *e.children.at(0)->children.at(0)) == 1);
				b = 0x31; w.u8(b); // i64.load8_u
				w.skip(2); // memflags + offset
			}
			else
			{
				SOUP_ASSERT(compileExpression(m, w, *e.children.at(0)) == 1);
				/* First, zero-extend i8 to i32: The top bits should be zero, so nothing to do. */
				/* Then, zero-extend i32 to i64: */ b = 0xad; w.u8(b); // i64.extend_i32_u
			}
			return 1;
		}
		SOUP_UNREACHABLE;
	}

	void laWasmBackend::discard(StringWriter& w, int nres)
	{
		while (nres--)
		{
			uint8_t b = 0x1a; w.u8(b); // drop
		}
	}
}

#include "wasm.hpp"

#include <cstring> // memset

#include "alloc.hpp"
#include "bitutil.hpp"
#include "MemoryRefReader.hpp"
#include "Reader.hpp"

#define DEBUG_LOAD false
#define DEBUG_VM false

#if DEBUG_LOAD || DEBUG_VM
#include <iostream>
#include "string.hpp"
#endif

// Good resources:
// - https://webassembly.github.io/wabt/demo/wat2wasm/
// - https://github.com/sunfishcode/wasm-reference-manual/blob/master/WebAssembly.md

NAMESPACE_SOUP
{
	// WasmScript

	WasmScript::~WasmScript() noexcept
	{
		free(memory);
	}

	bool WasmScript::load(const std::string& data)
	{
		MemoryRefReader r(data);
		return load(r);
	}

	bool WasmScript::load(Reader& r)
	{
		uint32_t u;
		r.u32_le(u);
		if (u != 1836278016) // magic - '\0asm' in little-endian
		{
			return false;
		}
		r.u32_le(u);
		if (u != 1) // version
		{
			return false;
		}
		while (r.hasMore())
		{
			uint8_t section_type;
			r.u8(section_type);
			size_t section_size;
			r.oml(section_size);
			switch (section_type)
			{
			default:
#if DEBUG_LOAD
				std::cout << "Ignoring section of type " << (int)section_type << "\n";
#endif
				SOUP_IF_UNLIKELY (section_size == 0)
				{
					return false;
				}
				r.skip(section_size);
				break;

			case 1: // Type
				{
					size_t num_types;
					r.oml(num_types);
#if DEBUG_LOAD
					std::cout << num_types << " type(s)\n";
#endif
					while (num_types--)
					{
						uint8_t type_type;
						r.u8(type_type);
						switch (type_type)
						{
						default:
							return false;

						case 0x60: // func
							{
#if DEBUG_LOAD
								std::cout << "- function with ";
#endif
								uint32_t num_parameters; r.oml(num_parameters); r.skip(num_parameters);
#if DEBUG_LOAD
								std::cout << num_parameters << " parameter(s) and ";
#endif
								uint32_t num_results; r.oml(num_results); r.skip(num_results);
#if DEBUG_LOAD
								std::cout << num_results << " return value(s)\n";
#endif
								types.emplace_back(FunctionType{ num_parameters, num_results });
							}
							break;
						}
					}
				}
				break;

			case 2: // Import
				{
					size_t num_imports;
					r.oml(num_imports);
#if DEBUG_LOAD
					std::cout << num_imports << " import(s)\n";
#endif
					while (num_imports--)
					{
						size_t module_name_len; r.oml(module_name_len);
						std::string module_name; r.str(module_name_len, module_name);
						size_t field_name_len; r.oml(field_name_len);
						std::string field_name; r.str(field_name_len, field_name);
#if DEBUG_LOAD
						std::cout << "-" << module_name << ":" << field_name << "\n";
#endif
						uint8_t kind; r.u8(kind);
						if (kind == 0) // function
						{
							size_t type_index; r.oml(type_index);
							function_imports.emplace_back(FunctionImport{ std::move(module_name), std::move(field_name), nullptr });
						}
					}
				}
				break;

			case 3: // Function
				{
					size_t num_functions;
					r.oml(num_functions);
#if DEBUG_LOAD
					std::cout << num_functions << " function(s)\n";
#endif
					while (num_functions--)
					{
						uint32_t type_index;
						r.oml(type_index);
						functions.emplace_back(type_index);
					}
				}
				break;

				// 4 - Table

			case 5: // Memory
				{
					size_t num_memories; r.oml(num_memories);
					SOUP_IF_UNLIKELY (memory != nullptr || num_memories != 1)
					{
						return false;
					}
					uint8_t flags; r.u8(flags);
					size_t pages; r.oml(pages);
					if (flags & 1)
					{
						size_t max_pages; r.oml(max_pages);
					}
					if (flags & 4)
					{
						memory64 = true;
					}
					if (pages == 0)
					{
						++pages;
					}
					memory = (uint8_t*)soup::malloc(pages * 0x10'000);
					memory_size = pages * 0x10'000;
					memset(memory, 0, memory_size);
#if DEBUG_LOAD
					std::cout << "Memory consists of " << pages << " pages, totalling " << memory_size << " bytes\n";
#endif
				}
				break;

			case 6: // Global
				{
					size_t num_globals;
					r.oml(num_globals);
					while (num_globals--)
					{
						uint8_t type; r.u8(type);
						SOUP_IF_UNLIKELY (type != 0x7f) // i32
						{
							return false;
						}
						r.skip(1); // mutability
						uint8_t op;
						r.u8(op);
						SOUP_IF_UNLIKELY (op != 0x41) // i32.const
						{
							return false;
						}
						int32_t value; r.soml(value);
						r.u8(op);
						SOUP_IF_UNLIKELY (op != 0x0b) // end
						{
							return false;
						}
						globals.emplace_back(value);
					}
				}
				break;

			case 7: // Export
				{
					size_t num_exports;
					r.oml(num_exports);
#if DEBUG_LOAD
					std::cout << num_exports << " export(s)\n";
#endif
					while (num_exports--)
					{
						size_t name_len;
						r.oml(name_len);
						std::string name;
						r.str(name_len, name);
#if DEBUG_LOAD
						std::cout << "- " << name << "\n";
#endif
						uint8_t kind; r.u8(kind);
						uint32_t index; r.oml(index);
						if (kind == 0) // function 
						{
							export_map.emplace(std::move(name), index);
						}
						// kind 2 = memory
					}
				}
				break;

			case 8: // Start
#if DEBUG_LOAD
				std::cout << "module has a start function\n";
#endif
				return false;

			case 9: // Elem
				{
					size_t num_segments;
					r.oml(num_segments);
#if DEBUG_LOAD
					std::cout << num_segments << " element segment(s)\n";
#endif
					while (num_segments--)
					{
						r.skip(1); // segment flags
						uint8_t op;
						r.u8(op);
						SOUP_IF_UNLIKELY (op != 0x41) // i32.const
						{
							return false;
						}
						int32_t base; r.soml(base);
						while (base > elements.size())
						{
							elements.emplace_back(-1);
						}
						r.u8(op);
						SOUP_IF_UNLIKELY (op != 0x0b) // end
						{
							return false;
						}
						size_t num_elements;
						r.oml(num_elements);
						while (num_elements--)
						{
							uint32_t function_index;
							r.oml(function_index);
							elements.emplace_back(function_index);
						}
					}
				}
				break;

			case 10: // Code
				{
					size_t num_functions;
					r.oml(num_functions);
#if DEBUG_LOAD
					std::cout << num_functions << " function(s)\n";
#endif
					while (num_functions--)
					{
						size_t body_size;
						r.oml(body_size);
						SOUP_IF_UNLIKELY (body_size == 0)
						{
							return false;
						}
						std::string body;
						r.str(body_size, body);
#if DEBUG_LOAD
						std::cout << "- " << string::bin2hex(body) << "\n";
#endif
						code.emplace_back(std::move(body));
					}
				}
				break;

			case 11: // Data
				{
					size_t num_segments;
					r.oml(num_segments);
#if DEBUG_LOAD
					std::cout << num_segments << " data segment(s)\n";
#endif
					while (num_segments--)
					{
						r.skip(1); // flags
						uint8_t op;
						r.u8(op);
						SOUP_IF_UNLIKELY (op != 0x41) // i32.const
						{
							return false;
						}
						int32_t base; r.soml(base);
						r.u8(op);
						SOUP_IF_UNLIKELY (op != 0x0b) // end
						{
							return false;
						}
						size_t size; r.oml(size);
						SOUP_IF_UNLIKELY (base + size >= memory_size)
						{
							return false;
						}
						r.raw(&memory[base], size);
					}
				}
				break;
			}
			if (section_size == 0)
			{
				r.oml(section_size);
			}
		}
		return true;
	}

	WasmScript::FunctionImport* WasmScript::getImportedFunction(const std::string& module_name, const std::string& function_name) noexcept
	{
		for (auto& fi : function_imports)
		{
			if (fi.function_name == function_name
				&& fi.module_name == module_name
				)
			{
				return &fi;
			}
		}
		return nullptr;
	}

	const std::string* WasmScript::getExportedFuntion(const std::string& name) const noexcept
	{
		if (auto e = export_map.find(name); e != export_map.end())
		{
			const size_t i = (e->second - function_imports.size());
			if (i < code.size())
			{
				return &code.at(i);
			}
		}
		return nullptr;
	}

	size_t WasmScript::allocateMemory(size_t len) noexcept
	{
		if (last_alloc >= memory_size)
		{
			last_alloc = memory_size - 1;
		}
		last_alloc -= len;
		return last_alloc;
	}

	bool WasmScript::setMemory(size_t ptr, const void* src, size_t len) noexcept
	{
		SOUP_IF_UNLIKELY (ptr + len >= memory_size)
		{
			return false;
		}
		memcpy(&memory[ptr], src, len);
		return true;
	}

	bool WasmScript::setMemory(WasmValue ptr, const void* src, size_t len) noexcept
	{
		return memory64
			? setMemory(static_cast<uint64_t>(ptr.i64), src, len)
			: setMemory(static_cast<uint32_t>(ptr.i32), src, len)
			;
	}

	void WasmScript::linkWasiPreview1() noexcept
	{
		// Resources:
		// - Barebones "Hello World": https://github.com/bytecodealliance/wasmtime/blob/main/docs/WASI-tutorial.md#web-assembly-text-example
		// - How the XCC compiler uses WASI:
		//   - https://github.com/tyfkda/xcc/blob/main/libsrc/_wasm/wasi.h
		//   - https://github.com/tyfkda/xcc/blob/main/libsrc/_wasm/crt0/_start.c#L41

		if (auto fi = getImportedFunction("wasi_snapshot_preview1", "args_sizes_get"))
		{
			fi->ptr = [](WasmVm& vm)
			{
				auto plen = vm.stack.top(); vm.stack.pop();
				auto pargc = vm.stack.top(); vm.stack.pop();
				*vm.script.getMemory<int32_t>(plen.i32) = sizeof("program");
				*vm.script.getMemory<int32_t>(pargc.i32) = 0;
				vm.stack.push(0);
			};
		}
		if (auto fi = getImportedFunction("wasi_snapshot_preview1", "args_get"))
		{
			fi->ptr = [](WasmVm& vm)
			{
				auto pstr = vm.stack.top(); vm.stack.pop();
				auto pargv = vm.stack.top(); vm.stack.pop();
				vm.script.setMemory(pstr.i32, "program", sizeof("program"));
				*vm.script.getMemory<int32_t>(pargv.i32) = pstr.i32;
				vm.stack.push(0);
			};
		}
		if (auto fi = getImportedFunction("wasi_snapshot_preview1", "proc_exit"))
		{
			fi->ptr = [](WasmVm& vm)
			{
				auto code = vm.stack.top(); vm.stack.pop();
				exit(code.i32);
			};
		}

		if (auto fi = getImportedFunction("wasi_snapshot_preview1", "fd_prestat_get"))
		{
			fi->ptr = [](WasmVm& vm)
			{
				auto prestat = vm.stack.top(); vm.stack.pop();
				auto fd = vm.stack.top(); vm.stack.pop();
				SOUP_UNUSED(prestat);
				SOUP_UNUSED(fd);
				vm.stack.push(-1);
			};
		}

		if (auto fi = getImportedFunction("wasi_snapshot_preview1", "fd_write"))
		{
			fi->ptr = [](WasmVm& vm)
			{
				auto out_nwritten = vm.stack.top(); vm.stack.pop();
				auto iovs_len = vm.stack.top(); vm.stack.pop();
				auto iovs = vm.stack.top(); vm.stack.pop();
				auto file_descriptor = vm.stack.top(); vm.stack.pop();
				auto nwritten = 0;
				if (file_descriptor.i32 == 1) // stdout
				{
					while (iovs_len.i32--)
					{
						int32_t iov_base = *vm.script.getMemory<int32_t>(iovs.i32); iovs.i32 += 4;
						int32_t iov_len = *vm.script.getMemory<int32_t>(iovs.i32); iovs.i32 += 4;
						fwrite(vm.script.getMemory<char>(iov_base), 1, iov_len, stdout);
						nwritten += iov_len;
					}
				}
				*vm.script.getMemory<int32_t>(out_nwritten.i32) = nwritten;
				vm.stack.push(nwritten);
			};
		}
		if (auto fi = getImportedFunction("wasi_snapshot_preview1", "fd_filestat_get"))
		{
			fi->ptr = [](WasmVm& vm)
			{
				auto out = vm.stack.top(); vm.stack.pop();
				auto fd = vm.stack.top(); vm.stack.pop();
				SOUP_UNUSED(out);
				SOUP_UNUSED(fd);
				vm.stack.push(fd.i32 < 3 ? 0 : -1);
			};
		}
	}

	size_t WasmScript::readUPTR(Reader& r) const noexcept
	{
		if (memory64)
		{
			uint64_t ptr;
			r.oml(ptr);
			return static_cast<size_t>(ptr);
		}
		uint32_t ptr;
		r.oml(ptr);
		return static_cast<size_t>(ptr);
	}

	// WasmVm

	bool WasmVm::run(const std::string& data) SOUP_EXCAL
	{
		MemoryRefReader r(data);
		return run(r);
	}

	bool WasmVm::run(Reader& r) SOUP_EXCAL
	{
		size_t local_decl_count;
		r.oml(local_decl_count);
		while (local_decl_count--)
		{
			size_t type_count;
			r.oml(type_count);
			uint8_t type;
			r.u8(type);
			SOUP_UNUSED(type);
			// type 0x7f = i32
			// type 0x7e = i64
			while (type_count--)
			{
				locals.emplace_back();
			}
		}

		std::stack<CtrlFlowEntry> ctrlflow{};

		uint8_t op;
		while (r.u8(op))
		{
			switch (op)
			{
			default:
#if DEBUG_VM
				std::cout << "Unsupported opcode: " << string::hex(op) << "\n";
#endif
				return false;

			case 0x00: // unreachable
#if DEBUG_VM
				std::cout << "unreachable\n";
#endif
				return false;

			case 0x01: // nop
				break;

			case 0x02: // block
				{
					uint8_t result_type; r.u8(result_type);
					bool has_result = (result_type != /* void */ 0x40);
					ctrlflow.emplace(CtrlFlowEntry{ (size_t)-1, stack.size() + has_result });
#if DEBUG_VM
					std::cout << "block at position " << r.getPosition() << " with stack size " << stack.size() << "\n";
#endif
				}
				break;

			case 0x03: // loop
				{
					uint8_t result_type; r.u8(result_type);
					bool has_result = (result_type != /* void */ 0x40);
					ctrlflow.emplace(CtrlFlowEntry{ r.getPosition(), stack.size() + has_result });
#if DEBUG_VM
					std::cout << "loop at position " << r.getPosition() << " with stack size " << stack.size() << "\n";
#endif
				}
				break;

			case 0x04: // if
				{
					uint8_t result_type; r.u8(result_type);
					bool has_result = (result_type != /* void */ 0x40);
					auto value = stack.top(); stack.pop();
					//std::cout << "if: condition is " << (value.i32 ? "true" : "false") << "\n";
					if (value.i32)
					{
						ctrlflow.emplace(CtrlFlowEntry{ r.getPosition(), stack.size() + has_result });
					}
					else
					{
						if (skipOverBranch(r))
						{
							// we're in the 'else' branch
							ctrlflow.emplace(CtrlFlowEntry{ r.getPosition(), stack.size() + has_result });
						}
						else
						{
							// we're after the 'end'
							SOUP_IF_UNLIKELY (has_result)
							{
#if DEBUG_VM
								std::cout << "if: has result but no 'else' clause\n";
#endif
								return false;
							}
						}
					}
				}
				break;

			case 0x05: // else
				//std::cout << "else: skipping over this branch\n";
				skipOverBranch(r);
				[[fallthrough]];
			case 0x0b: // end
				if (ctrlflow.empty())
				{
					return true;
				}
				ctrlflow.pop();
				//std::cout << "ctrlflow stack now has " << ctrlflow.size() << " entries\n";
				break;

			case 0x0c: // br
				{
					uint32_t depth;
					r.oml(depth);
					SOUP_IF_UNLIKELY (!doBranch(r, depth, ctrlflow))
					{
						return false;
					}
				}
				break;

			case 0x0d: // br_if
				{
					uint32_t depth;
					r.oml(depth);
					auto value = stack.top(); stack.pop();
					if (value.i32)
					{
						SOUP_IF_UNLIKELY (!doBranch(r, depth, ctrlflow))
						{
							return false;
						}
					}
				}
				break;

			case 0x0e: // br_table
				{
					std::vector<uint32_t> table;
					uint32_t num_branches;
					r.oml(num_branches);
					table.reserve(num_branches);
					while (num_branches--)
					{
						uint32_t depth;
						r.oml(depth);
						table.emplace_back(depth);
					}
					uint32_t depth;
					r.oml(depth);
					auto index = static_cast<uint32_t>(stack.top().i32); stack.pop();
					if (index < table.size())
					{
						depth = table.at(index);
					}
					SOUP_IF_UNLIKELY (!doBranch(r, depth, ctrlflow))
					{
						return false;
					}
				}
				break;

			case 0x0f: // return
				return true;

			case 0x10: // call
				{
					uint32_t function_index;
					r.oml(function_index);
					if (function_index < script.function_imports.size())
					{
#if DEBUG_VM
						std::cout << "Calling into " << script.function_imports.at(function_index).module_name << ":" << script.function_imports.at(function_index).function_name << "\n";
#endif
						SOUP_IF_UNLIKELY (script.function_imports.at(function_index).ptr == nullptr)
						{
#if DEBUG_VM
							std::cout << "call: function is not imported\n";
#endif
							return false;
						}
						script.function_imports.at(function_index).ptr(*this);
						break;
					}
					function_index -= static_cast<uint32_t>(script.function_imports.size());
					SOUP_IF_UNLIKELY (function_index >= script.functions.size() || function_index >= script.code.size())
					{
#if DEBUG_VM
						std::cout << "call: function is out-of-bounds\n";
#endif
						return false;
					}
					uint32_t type_index = script.functions.at(function_index);
					SOUP_IF_UNLIKELY (!doCall(type_index, function_index))
					{
						return false;
					}
				}
				break;

			case 0x11: // call_indirect
				{
					uint32_t type_index; r.oml(type_index);
					uint32_t table_index; r.oml(table_index);
					SOUP_IF_UNLIKELY (table_index != 0)
					{
#if DEBUG_VM
						std::cout << "call: table is out-of-bounds\n";
#endif
						return false;
					}
					auto element_index = static_cast<uint32_t>(stack.top().i32); stack.pop();
					SOUP_IF_UNLIKELY (element_index >= script.elements.size())
					{
#if DEBUG_VM
						std::cout << "call: element is out-of-bounds\n";
#endif
						return false;
					}
					uint32_t function_index = script.elements.at(element_index);
					SOUP_IF_UNLIKELY (function_index < script.function_imports.size())
					{
#if DEBUG_VM
						std::cout << "indirect call to imported function\n";
#endif
						return false;
					}
					function_index -= static_cast<uint32_t>(script.function_imports.size());
					SOUP_IF_UNLIKELY (type_index != script.functions.at(function_index))
					{
#if DEBUG_VM
						std::cout << "call: function type mismatch\n";
#endif
						return false;
					}
					SOUP_RETHROW_FALSE(doCall(type_index, function_index));
				}
				break;

			case 0x1a: // drop
				stack.pop();
				break;

			case 0x1b: // select
				{
					auto cond = stack.top(); stack.pop();
					auto fvalue = stack.top(); stack.pop();
					auto tvalue = stack.top(); stack.pop();
					stack.push(cond.i32 ? tvalue : fvalue);
				}
				break;

			case 0x20: // local.get
				{
					size_t local_index;
					r.oml(local_index);
					SOUP_IF_UNLIKELY (local_index >= locals.size())
					{
#if DEBUG_VM
						std::cout << "local.get: index is out-of-bounds: " << local_index << "\n";
#endif
						return false;
					}
					stack.push(locals.at(local_index));
				}
				break;

			case 0x21: // local.set
				{
					size_t local_index;
					r.oml(local_index);
					SOUP_IF_UNLIKELY (local_index >= locals.size())
					{
#if DEBUG_VM
						std::cout << "local.set: index is out-of-bounds: " << local_index << "\n";
#endif
						return false;
					}
					locals.at(local_index) = stack.top(); stack.pop();
				}
				break;

			case 0x22: // local.tee
				{
					size_t local_index;
					r.oml(local_index);
					SOUP_IF_UNLIKELY (local_index >= locals.size())
					{
#if DEBUG_VM
						std::cout << "local.tee: index is out-of-bounds: " << local_index << "\n";
#endif
						return false;
					}
					locals.at(local_index) = stack.top();
				}
				break;

			case 0x23: // global.get
				{
					size_t global_index;
					r.oml(global_index);
					SOUP_IF_UNLIKELY (global_index >= script.globals.size())
					{
#if DEBUG_VM
						std::cout << "global.get: index is out-of-bounds: " << global_index << "\n";
#endif
						return false;
					}
					stack.push(script.globals.at(global_index));
				}
				break;

			case 0x24: // global.set
				{
					size_t global_index;
					r.oml(global_index);
					SOUP_IF_UNLIKELY (global_index >= script.globals.size())
					{
#if DEBUG_VM
						std::cout << "global.set: index is out-of-bounds: " << global_index << "\n";
#endif
						return false;
					}
					script.globals.at(global_index) = stack.top().i32; stack.pop();
				}
				break;

			case 0x28: // i32.load
				{
					auto base = stack.top(); stack.pop();
					r.skip(1); // memflags
					auto offset = script.readUPTR(r);
					if (auto ptr = script.getMemory<int32_t>(base, offset))
					{
						stack.emplace(*ptr);
					}
					else
					{
#if DEBUG_VM
						std::cout << "memory access out of bounds\n";
#endif
						return false;
					}
				}
				break;

			case 0x29: // i64.load
				{
					auto base = stack.top(); stack.pop();
					r.skip(1); // memflags
					auto offset = script.readUPTR(r);
					if (auto ptr = script.getMemory<int64_t>(base, offset))
					{
						stack.emplace(*ptr);
					}
					else
					{
#if DEBUG_VM
						std::cout << "memory access out of bounds\n";
#endif
						return false;
					}
				}
				break;

			case 0x2a: // f32.load
				{
					auto base = stack.top(); stack.pop();
					r.skip(1); // memflags
					auto offset = script.readUPTR(r);
					if (auto ptr = script.getMemory<float>(base, offset))
					{
						stack.emplace(*ptr);
					}
					else
					{
#if DEBUG_VM
						std::cout << "memory access out of bounds\n";
#endif
						return false;
					}
				}
				break;

			case 0x2b: // f64.load
				{
					auto base = stack.top(); stack.pop();
					r.skip(1); // memflags
					auto offset = script.readUPTR(r);
					if (auto ptr = script.getMemory<double>(base, offset))
					{
						stack.emplace(*ptr);
					}
					else
					{
#if DEBUG_VM
						std::cout << "memory access out of bounds\n";
#endif
						return false;
					}
				}
				break;

			case 0x2c: // i32.load8_s
				{
					auto base = stack.top(); stack.pop();
					r.skip(1); // memflags
					auto offset = script.readUPTR(r);
					if (auto ptr = script.getMemory<int8_t>(base, offset))
					{
						stack.emplace(static_cast<int32_t>(*ptr));
					}
					else
					{
#if DEBUG_VM
						std::cout << "memory access out of bounds\n";
#endif
						return false;
					}
				}
				break;

			case 0x2d: // i32.load8_u
				{
					auto base = stack.top(); stack.pop();
					r.skip(1); // memflags
					auto offset = script.readUPTR(r);
					if (auto ptr = script.getMemory<uint8_t>(base, offset))
					{
						stack.emplace(static_cast<uint32_t>(*ptr));
					}
					else
					{
#if DEBUG_VM
						std::cout << "memory access out of bounds\n";
#endif
						return false;
					}
				}
				break;

			case 0x2e: // i32.load16_s
				{
					auto base = stack.top(); stack.pop();
					r.skip(1); // memflags
					auto offset = script.readUPTR(r);
					if (auto ptr = script.getMemory<int16_t>(base, offset))
					{
						stack.emplace(static_cast<uint32_t>(*ptr));
					}
					else
					{
#if DEBUG_VM
						std::cout << "memory access out of bounds\n";
#endif
						return false;
					}
				}
				break;

			case 0x2f: // i32.load16_u
				{
					auto base = stack.top(); stack.pop();
					r.skip(1); // memflags
					auto offset = script.readUPTR(r);
					if (auto ptr = script.getMemory<uint16_t>(base, offset))
					{
						stack.emplace(static_cast<uint32_t>(*ptr));
					}
					else
					{
#if DEBUG_VM
						std::cout << "memory access out of bounds\n";
#endif
						return false;
					}
				}
				break;

			case 0x30: // i64.load8_s
				{
					auto base = stack.top(); stack.pop();
					r.skip(1); // memflags
					auto offset = script.readUPTR(r);
					if (auto ptr = script.getMemory<int8_t>(base, offset))
					{
						stack.emplace(static_cast<int64_t>(*ptr));
					}
					else
					{
#if DEBUG_VM
						std::cout << "memory access out of bounds\n";
#endif
						return false;
					}
				}
				break;

			case 0x31: // i64.load8_u
				{
					auto base = stack.top(); stack.pop();
					r.skip(1); // memflags
					auto offset = script.readUPTR(r);
					if (auto ptr = script.getMemory<uint8_t>(base, offset))
					{
						stack.emplace(static_cast<uint64_t>(*ptr));
					}
					else
					{
#if DEBUG_VM
						std::cout << "memory access out of bounds\n";
#endif
						return false;
					}
				}
				break;

			case 0x32: // i64.load16_s
				{
					auto base = stack.top(); stack.pop();
					r.skip(1); // memflags
					auto offset = script.readUPTR(r);
					if (auto ptr = script.getMemory<int16_t>(base, offset))
					{
						stack.emplace(static_cast<int64_t>(*ptr));
					}
					else
					{
#if DEBUG_VM
						std::cout << "memory access out of bounds\n";
#endif
						return false;
					}
				}
				break;

			case 0x33: // i64.load16_u
				{
					auto base = stack.top(); stack.pop();
					r.skip(1); // memflags
					auto offset = script.readUPTR(r);
					if (auto ptr = script.getMemory<uint16_t>(base, offset))
					{
						stack.emplace(static_cast<uint64_t>(*ptr));
					}
					else
					{
#if DEBUG_VM
						std::cout << "memory access out of bounds\n";
#endif
						return false;
					}
				}
				break;

			case 0x34: // i64.load32_s
				{
					auto base = stack.top(); stack.pop();
					r.skip(1); // memflags
					auto offset = script.readUPTR(r);
					if (auto ptr = script.getMemory<int32_t>(base, offset))
					{
						stack.emplace(static_cast<int64_t>(*ptr));
					}
					else
					{
#if DEBUG_VM
						std::cout << "memory access out of bounds\n";
#endif
						return false;
					}
				}
				break;

			case 0x35: // i64.load32_u
				{
					auto base = stack.top(); stack.pop();
					r.skip(1); // memflags
					auto offset = script.readUPTR(r);
					if (auto ptr = script.getMemory<uint32_t>(base, offset))
					{
						stack.emplace(static_cast<uint64_t>(*ptr));
					}
					else
					{
#if DEBUG_VM
						std::cout << "memory access out of bounds\n";
#endif
						return false;
					}
				}
				break;


			case 0x36: // i32.store
				{
					auto value = stack.top(); stack.pop();
					auto base = stack.top(); stack.pop();
					r.skip(1); // memflags
					auto offset = script.readUPTR(r);
					if (auto ptr = script.getMemory<int32_t>(base, offset))
					{
						*ptr = value.i32;
					}
					else
					{
#if DEBUG_VM
						std::cout << "memory access out of bounds\n";
#endif
						return false;
					}
				}
				break;

			case 0x37: // i64.store
				{
					auto value = stack.top(); stack.pop();
					auto base = stack.top(); stack.pop();
					r.skip(1); // memflags
					auto offset = script.readUPTR(r);
					if (auto ptr = script.getMemory<int64_t>(base, offset))
					{
						*ptr = value.i64;
					}
					else
					{
#if DEBUG_VM
						std::cout << "memory access out of bounds\n";
#endif
						return false;
					}
				}
				break;

			case 0x38: // f32.store
				{
					auto value = stack.top(); stack.pop();
					auto base = stack.top(); stack.pop();
					r.skip(1); // memflags
					auto offset = script.readUPTR(r);
					*script.getMemory<float>(base, offset) = value.f32;
				}
				break;

			case 0x39: // f64.store
				{
					auto value = stack.top(); stack.pop();
					auto base = stack.top(); stack.pop();
					r.skip(1); // memflags
					auto offset = script.readUPTR(r);
					*script.getMemory<double>(base, offset) = value.f64;
				}
				break;

			case 0x3a: // i32.store8
				{
					auto value = stack.top(); stack.pop();
					auto base = stack.top(); stack.pop();
					r.skip(1); // memflags
					auto offset = script.readUPTR(r);
					*script.getMemory<int8_t>(base, offset) = static_cast<int8_t>(value.i32);
				}
				break;

			case 0x3b: // i32.store16
				{
					auto value = stack.top(); stack.pop();
					auto base = stack.top(); stack.pop();
					r.skip(1); // memflags
					auto offset = script.readUPTR(r);
					*script.getMemory<int16_t>(base, offset) = static_cast<int16_t>(value.i32);
				}
				break;

			case 0x3f: // memory.size
				{
					r.skip(1); // reserved
					pushIPTR(script.memory_size / 0x10'000);
				}
				break;

			case 0x40: // memory.grow
				{
					r.skip(1); // reserved
					size_t delta;
					if (script.memory64)
					{
						delta = static_cast<uint64_t>(stack.top().i64);
					}
					else
					{
						delta = static_cast<uint32_t>(stack.top().i32);
					}
					stack.pop();
					delta *= 0x10'000;
					auto nmem = (uint8_t*)::realloc(script.memory, script.memory_size + delta);
					if (nmem == nullptr)
					{
						pushIPTR(-1);
					}
					else
					{
						memset(&nmem[script.memory_size], 0, delta);
						pushIPTR(script.memory_size / 0x10'000);
						script.memory = nmem;
						script.memory_size += delta;
					}
				}
				break;

			case 0x41: // i32.const
				{
					int32_t value;
					r.soml(value);
					stack.push(value);
				}
				break;

			case 0x42: // i64.const
				{
					int64_t value;
					r.soml(value);
					stack.push(value);
				}
				break;

			case 0x43: // f32.const
				{
					float value;
					r.f32(value);
					stack.push(value);
				}
				break;

			case 0x44: // f64.const
				{
					double value;
					r.f64(value);
					stack.push(value);
				}
				break;

			case 0x45: // i32.eqz
				{
					auto value = stack.top(); stack.pop();
					stack.push(value.i32 == 0);
				}
				break;

			case 0x46: // i32.eq
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.i32 == b.i32);
				}
				break;

			case 0x47: // i32.ne
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.i32 != b.i32);
				}
				break;

			case 0x48: // i32.lt_s
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.i32 < b.i32);
				}
				break;

			case 0x49: // i32.lt_u
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(static_cast<uint32_t>(a.i32) < static_cast<uint32_t>(b.i32));
				}
				break;

			case 0x4a: // i32.gt_s
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.i32 > b.i32);
				}
				break;

			case 0x4b: // i32.gt_u
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(static_cast<uint32_t>(a.i32) > static_cast<uint32_t>(b.i32));
				}
				break;

			case 0x4c: // i32.le_s
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.i32 <= b.i32);
				}
				break;

			case 0x4d: // i32.le_u
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(static_cast<uint32_t>(a.i32) <= static_cast<uint32_t>(b.i32));
				}
				break;

			case 0x4e: // i32.ge_s
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.i32 >= b.i32);
				}
				break;

			case 0x4f: // i32.ge_u
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(static_cast<uint32_t>(a.i32) >= static_cast<uint32_t>(b.i32));
				}
				break;

			case 0x50: // i64.eqz
				{
					auto value = stack.top(); stack.pop();
					stack.push(value.i64 == 0);
				}
				break;

			case 0x51: // i64.eq
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.i64 == b.i64);
				}
				break;

			case 0x52: // i64.ne
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.i64 != b.i64);
				}
				break;

			case 0x53: // i64.lt_s
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.i64 < b.i64);
				}
				break;

			case 0x54: // i64.lt_u
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(static_cast<uint64_t>(a.i64) < static_cast<uint64_t>(b.i64));
				}
				break;

			case 0x55: // i64.gt_s
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.i64 > b.i64);
				}
				break;

			case 0x56: // i64.gt_u
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(static_cast<uint64_t>(a.i64) > static_cast<uint64_t>(b.i64));
				}
				break;

			case 0x57: // i64.le_s
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.i64 <= b.i64);
				}
				break;

			case 0x58: // i64.le_u
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(static_cast<uint64_t>(a.i64) <= static_cast<uint64_t>(b.i64));
				}
				break;

			case 0x59: // i64.ge_s
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.i64 >= b.i64);
				}
				break;

			case 0x5a: // i64.ge_u
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(static_cast<uint64_t>(a.i64) >= static_cast<uint64_t>(b.i64));
				}
				break;

			case 0x5b: // f32.eq
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.f32 == b.f32);
				}
				break;

			case 0x5c: // f32.ne
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.f32 != b.f32);
				}
				break;

			case 0x5d: // f32.lt
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.f32 < b.f32);
				}
				break;

			case 0x5e: // f32.gt
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.f32 > b.f32);
				}
				break;

			case 0x5f: // f32.le
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.f32 <= b.f32);
				}
				break;

			case 0x60: // f32.ge
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.f32 >= b.f32);
				}
				break;

			case 0x61: // f64.eq
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.f64 == b.f64);
				}
				break;

			case 0x62: // f64.ne
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.f64 != b.f64);
				}
				break;

			case 0x63: // f64.lt
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.f64 < b.f64);
				}
				break;

			case 0x64: // f64.gt
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.f64 > b.f64);
				}
				break;

			case 0x65: // f64.le
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.f64 <= b.f64);
				}
				break;

			case 0x66: // f64.ge
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.f64 >= b.f64);
				}
				break;

			case 0x69: // i32.popcnt
				{
					auto value = stack.top(); stack.pop();
					stack.push(bitutil::getNumSetBits(static_cast<uint32_t>(value.i32)));
				}
				break;

			case 0x6a: // i32.add
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.i32 + b.i32);
				}
				break;

			case 0x6b: // i32.sub
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.i32 - b.i32);
				}
				break;

			case 0x6c: // i32.mul
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.i32 * b.i32);
				}
				break;

			case 0x6d: // i32.div_s
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					SOUP_IF_UNLIKELY (b.i32 == 0)
					{
						return 0;
					}
					stack.push(a.i32 / b.i32);
				}
				break;

			case 0x6e: // i32.div_u
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					SOUP_IF_UNLIKELY (b.i32 == 0)
					{
						return 0;
					}
					stack.push(static_cast<uint32_t>(a.i32) / static_cast<uint32_t>(b.i32));
				}
				break;

			case 0x6f: // i32.rem_s
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.i32 % b.i32);
				}
				break;

			case 0x70: // i32.rem_u
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(static_cast<uint32_t>(a.i32) % static_cast<uint32_t>(b.i32));
				}
				break;

			case 0x71: // i32.and
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.i32 & b.i32);
				}
				break;

			case 0x72: // i32.or
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.i32 | b.i32);
				}
				break;

			case 0x73: // i32.xor
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.i32 ^ b.i32);
				}
				break;

			case 0x74: // i32.shl
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.i32 << (static_cast<uint32_t>(b.i32) % (sizeof(uint32_t) * 8)));
				}
				break;

			case 0x75: // i32.shr_s ("arithmetic right shift")
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.i32 >> (static_cast<uint32_t>(b.i32) % (sizeof(uint32_t) * 8)));
				}
				break;

			case 0x76: // i32.shr_u ("logical right shift")
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(static_cast<uint32_t>(a.i32) >> (static_cast<uint32_t>(b.i32) % (sizeof(uint32_t) * 8)));
				}
				break;

			case 0x7c: // i64.add
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.i64 + b.i64);
				}
				break;

			case 0x7d: // i64.sub
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.i64 - b.i64);
				}
				break;

			case 0x7e: // i64.mul
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.i64 * b.i64);
				}
				break;

			case 0x7f: // i64.div_s
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					SOUP_IF_UNLIKELY (b.i64 == 0)
					{
						return 0;
					}
					stack.push(a.i64 / b.i64);
				}
				break;

			case 0x80: // i64.div_u
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					SOUP_IF_UNLIKELY (b.i64 == 0)
					{
						return 0;
					}
					stack.push(static_cast<uint64_t>(a.i64) / static_cast<uint64_t>(b.i64));
				}
				break;

			case 0x81: // i64.rem_s
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.i64 % b.i64);
				}
				break;

			case 0x82: // i64.rem_u
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(static_cast<uint64_t>(a.i64) % static_cast<uint64_t>(b.i64));
				}
				break;

			case 0x83: // i64.and
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.i64 & b.i64);
				}
				break;

			case 0x84: // i64.or
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.i64 | b.i64);
				}
				break;

			case 0x85: // i64.xor
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.i64 ^ b.i64);
				}
				break;

			case 0x86: // i64.shl
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.i64 << (static_cast<uint64_t>(b.i64) % (sizeof(uint64_t) * 8)));
				}
				break;

			case 0x87: // i64.shr_s ("arithmetic right shift")
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.i64 >> (static_cast<uint64_t>(b.i64) % (sizeof(uint64_t) * 8)));
				}
				break;

			case 0x88: // i64.shr_u ("logical right shift")
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(static_cast<uint64_t>(a.i64) >> (static_cast<uint64_t>(b.i64) % (sizeof(uint64_t) * 8)));
				}
				break;

			case 0x92: // f32.add
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.f32 + b.f32);
				}
				break;

			case 0x93: // f32.sub
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.f32 - b.f32);
				}
				break;

			case 0x94: // f32.mul
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.f32 * b.f32);
				}
				break;

			case 0x95: // f32.mul
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.f32 / b.f32);
				}
				break;

			case 0xa0: // f64.add
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.f64 + b.f64);
				}
				break;

			case 0xa1: // f64.sub
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.f64 - b.f64);
				}
				break;

			case 0xa2: // f64.mul
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.f64 * b.f64);
				}
				break;

			case 0xa3: // f64.mul
				{
					auto b = stack.top(); stack.pop();
					auto a = stack.top(); stack.pop();
					stack.push(a.f64 / b.f64);
				}
				break;

			case 0xa7: // i32.wrap_i64
				{
					auto value = stack.top(); stack.pop();
					stack.push(static_cast<int32_t>(value.i64));
				}
				break;

			case 0xac: // i64.extend_i32_s
				{
					auto value = stack.top(); stack.pop();
					stack.push(static_cast<int64_t>(value.i32));
				}
				break;

			case 0xad: // i64.extend_i32_u
				{
					auto value = stack.top(); stack.pop();
					stack.push(static_cast<int64_t>(static_cast<uint64_t>(static_cast<uint32_t>(value.i32))));
				}
				break;
			}
		}
		return true;
	}

	bool WasmVm::skipOverBranch(Reader& r, uint32_t depth) SOUP_EXCAL
	{
		uint8_t op;
		while (r.u8(op))
		{
			switch (op)
			{
			case 0x02: // block
			case 0x03: // loop
			case 0x04: // if
				r.skip(1); // result type
				++depth;
				break;

			case 0x05: // else
				if (depth == 0)
				{
					return true;
				}
				break;

			case 0x0b: // end
				if (depth == 0)
				{
					return false;
				}
				--depth;
				break;

			case 0x0c: // br
			case 0x0d: // br_if
			case 0x10: // call
			case 0x20: // local.get
			case 0x21: // local.set
			case 0x22: // local.tee
			case 0x23: // global.get
			case 0x24: // global.set
				{
					size_t imm;
					r.oml(imm);
				}
				break;

			case 0x0e: // br_table
				{
					uint32_t num_branches;
					r.oml(num_branches);
					while (num_branches--)
					{
						uint32_t depth;
						r.oml(depth);
					}
					uint32_t default_depth;
					r.oml(default_depth);
				}
				break;

			case 0x11: // call_indirect
				{
					uint32_t type_index; r.oml(type_index);
					uint32_t table_index; r.oml(table_index);
				}
				break;

			case 0x28: // i32.load
			case 0x29: // i64.load
			case 0x2a: // f32.load
			case 0x2b: // f64.load
			case 0x2c: // i32.load8_s
			case 0x2d: // i32.load8_u
			case 0x2e: // i32.load16_s
			case 0x2f: // i32.load16_u
			case 0x30: // i64.load8_s
			case 0x31: // i64.load8_u
			case 0x32: // i64.load16_s
			case 0x33: // i64.load16_u
			case 0x34: // i64.load32_s
			case 0x35: // i64.load32_u
			case 0x36: // i32.store
			case 0x37: // i64.store
			case 0x38: // f32.store
			case 0x39: // f64.store
			case 0x3a: // i32.store8
			case 0x3b: // i32.store16
				{
					r.skip(1); // memflags
					SOUP_UNUSED(script.readUPTR(r));
				}
				break;

			case 0x3f: // memory.size
			case 0x40: // memory.grow
				r.skip(1);
				break;

			case 0x41: // i32.const
				{
					int32_t value;
					r.soml(value);
				}
				break;

			case 0x42: // i64.const
				{
					int64_t value;
					r.soml(value);
				}
				break;

			case 0x43: // f32.const
				r.skip(4);
				break;

			case 0x44: // f64.const
				r.skip(8);
				break;

#if DEBUG_VM
			case 0x00: // unreachable
			case 0x01: // nop
			case 0x0f: // return
			case 0x1a: // drop
			case 0x1b: // select
			case 0x45: // i32.eqz
			case 0x46: // i32.eq
			case 0x47: // i32.ne
			case 0x48: // i32.lt_s
			case 0x49: // i32.lt_u
			case 0x4a: // i32.gt_s
			case 0x4b: // i32.gt_u
			case 0x4c: // i32.le_s
			case 0x4d: // i32.le_u
			case 0x4e: // i32.ge_s
			case 0x4f: // i32.ge_u
			case 0x50: // i64.eqz
			case 0x51: // i64.eq
			case 0x52: // i64.ne
			case 0x53: // i64.lt_s
			case 0x54: // i64.lt_u
			case 0x55: // i64.gt_s
			case 0x56: // i64.gt_u
			case 0x57: // i64.le_s
			case 0x58: // i64.le_u
			case 0x59: // i64.ge_s
			case 0x5a: // i64.ge_u
			case 0x5b: // f32.eq
			case 0x5c: // f32.ne
			case 0x5d: // f32.lt
			case 0x5e: // f32.gt
			case 0x5f: // f32.le
			case 0x60: // f32.ge
			case 0x61: // f64.eq
			case 0x62: // f64.ne
			case 0x63: // f64.lt
			case 0x64: // f64.gt
			case 0x65: // f64.le
			case 0x66: // f64.ge
			case 0x69: // i32.popcnt
			case 0x6a: // i32.add
			case 0x6b: // i32.sub
			case 0x6c: // i32.mul
			case 0x6d: // i32.div_s
			case 0x6e: // i32.div_u
			case 0x6f: // i32.rem_s
			case 0x70: // i32.rem_u
			case 0x71: // i32.and
			case 0x72: // i32.or
			case 0x73: // i32.xor
			case 0x74: // i32.shl
			case 0x75: // i32.shr_s ("arithmetic right shift")
			case 0x76: // i32.shr_u ("logical right shift")
			case 0x7c: // i64.add
			case 0x7d: // i64.sub
			case 0x7e: // i64.mul
			case 0x7f: // i64.div_s
			case 0x80: // i64.div_u
			case 0x81: // i64.rem_s
			case 0x82: // i64.rem_u
			case 0x83: // i64.and
			case 0x84: // i64.or
			case 0x85: // i64.xor
			case 0x86: // i64.shl
			case 0x87: // i64.shr_s ("arithmetic right shift")
			case 0x88: // i64.shr_u ("logical right shift")
			case 0x92: // f32.add
			case 0x93: // f32.sub
			case 0x94: // f32.mul
			case 0x95: // f32.mul
			case 0xa0: // f64.add
			case 0xa1: // f64.sub
			case 0xa2: // f64.mul
			case 0xa3: // f64.mul
			case 0xa7: // i32.wrap_i64
			case 0xac: // i64.extend_i32_s
			case 0xad: // i64.extend_i32_u
				break;

			default:
				std::cout << "skipOverBranch: unknown instruction " << string::hex(op) << ", might cause problems\n";
				break;
#endif
			}
		}
#if DEBUG_VM
		std::cout << "skipOverBranch: end of stream reached\n";
#endif
		return false;
	}

	bool WasmVm::doBranch(Reader& r, uint32_t depth, std::stack<CtrlFlowEntry>& ctrlflow) SOUP_EXCAL
	{
#if DEBUG_VM
		std::cout << "branch with depth " << depth << " at position " << r.getPosition() << "\n";
#endif
		SOUP_IF_UNLIKELY (ctrlflow.empty())
		{
#if DEBUG_VM
			std::cout << "branch returns from function\n";
#endif
			r.seekEnd();
			return true;
		}
		for (uint32_t i = 0; i != depth; ++i)
		{
			ctrlflow.pop();
			SOUP_IF_UNLIKELY (ctrlflow.empty())
			{
#if DEBUG_VM
				std::cout << "branch returns from function\n";
#endif
				r.seekEnd();
				return true;
			}
		}
		if (ctrlflow.top().position == -1)
		{
			// branch forwards
			skipOverBranch(r, depth);
		}
		else
		{
			// branch backwards
			r.seek(ctrlflow.top().position);
		}
#if DEBUG_VM
		std::cout << "position after branch: " << r.getPosition() << "\n";
#endif
		while (stack.size() > ctrlflow.top().stack_size)
		{
			stack.pop();
		}
		return true;
	}

	bool WasmVm::doCall(uint32_t type_index, uint32_t function_index) SOUP_EXCAL
	{
		SOUP_IF_UNLIKELY (type_index >= script.types.size())
		{
#if DEBUG_VM
			std::cout << "call: type is out-of-bounds\n";
#endif
			return false;
		}
		const auto& type = script.types.at(type_index);
		WasmVm callvm(script);
		for (uint32_t i = 0; i != type.num_parameters; ++i)
		{
			//std::cout << "arg: " << script.getMemory<const char>(stack.top()) << "\n";
			callvm.locals.insert(callvm.locals.begin(), stack.top()); stack.pop();
		}
#if DEBUG_VM
		std::cout << "call: enter " << function_index << "\n";
		//std::cout << string::bin2hex(script.code.at(function_index)) << "\n";
#endif
		SOUP_IF_UNLIKELY (!callvm.run(script.code.at(function_index)))
		{
			return false;
		}
#if DEBUG_VM
		std::cout << "call: leave " << function_index << "\n";
#endif
		if (type.num_results < 2)
		{
			for (uint32_t i = 0; i != type.num_results; ++i)
			{
				SOUP_IF_UNLIKELY (callvm.stack.empty())
				{
#if DEBUG_VM
					std::cout << "call: not enough values on the stack after return\n";
#endif
					return false;
				}
				//std::cout << "return value: " << callvm.stack.top() << "\n";
				stack.emplace(callvm.stack.top()); callvm.stack.pop();
			}
			SOUP_IF_UNLIKELY (!callvm.stack.empty())
			{
#if DEBUG_VM
				std::cout << "call: too many values on the stack after return\n";
#endif
				return false;
			}
		}
		else
		{
			std::vector<WasmValue> results{};
			results.reserve(type.num_results);
			for (uint32_t i = 0; i != type.num_results; ++i)
			{
				SOUP_IF_UNLIKELY (callvm.stack.empty())
				{
#if DEBUG_VM
					std::cout << "call: not enough values on the stack after return\n";
#endif
					return false;
				}
				//std::cout << "return value: " << callvm.stack.top() << "\n";
				results.emplace_back(callvm.stack.top()); callvm.stack.pop();
			}
			SOUP_IF_UNLIKELY (!callvm.stack.empty())
			{
#if DEBUG_VM
				std::cout << "call: too many values on the stack after return\n";
#endif
				return false;
			}
			for (auto i = results.rbegin(); i != results.rend(); ++i)
			{
				stack.emplace(std::move(*i));
			}
		}
		return true;
	}

	void WasmVm::pushIPTR(size_t ptr) SOUP_EXCAL
	{
		if (script.memory64)
		{
			stack.push(static_cast<uint64_t>(ptr));
		}
		else
		{
			stack.push(static_cast<uint32_t>(ptr));
		}
	}
}

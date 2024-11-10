#include "PhpState.hpp"

#include "LangDesc.hpp"
#include "LangVm.hpp"
#include "ParseError.hpp"
#include "ParserState.hpp"
#include "parse_tree.hpp"
#include "string.hpp"
#include "StringReader.hpp"
#include "StringWriter.hpp"
#include "UniquePtr.hpp"

NAMESPACE_SOUP
{
	enum PhpOpCodes : int
	{
		OP_CONCAT = 0,
		OP_INDEX,
		OP_ASSIGN,
		OP_EQ,
		OP_IF,
		OP_IF_ELSE,
		OP_CALL,
		OP_REQUIRE,
		OP_ECHO,
	};

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wstring-compare"
#endif
	[[nodiscard]] static LangDesc getLangDescImpl()
	{
		LangDesc ld;
		ld.addToken("<?php");
		ld.addToken("?>");
		ld.addBlock("{", "}");
		ld.addToken("function", Rgb::BLUE, [](ParserState& ps)
		{
			auto node = ps.popRighthand();
			if (node->type != astNode::LEXEME)
			{
				std::string err = "'function' expected righthand '(' or name, found ";
				err.append(node->toString());
				SOUP_THROW(ParseError(std::move(err)));
			}
			UniquePtr<astNode> var_name_literal{};
			if (static_cast<LexemeNode*>(node.get())->lexeme.token_keyword == Lexeme::LITERAL)
			{
				var_name_literal = std::move(node);
				node = ps.popRighthand();
			}
			if (static_cast<LexemeNode*>(node.get())->lexeme.token_keyword != "(")
			{
				std::string err = "'function' expected righthand '(', found ";
				err.append(node->toString());
				SOUP_THROW(ParseError(std::move(err)));
			}

			node = ps.popRighthand();
			if (node->type != astNode::LEXEME
				|| static_cast<LexemeNode*>(node.get())->lexeme.token_keyword != Lexeme::LITERAL
				)
			{
				std::string err = "'function' expected righthand ')' or parameter list, found ";
				err.append(node->toString());
				SOUP_THROW(ParseError(std::move(err)));
			}
			std::vector<UniquePtr<astNode>> param_literals{};
			if (static_cast<LexemeNode*>(node.get())->lexeme.val.getString() != ")")
			{
				while (true)
				{
					param_literals.emplace_back(std::move(node));

					node = ps.popRighthand();
					if (node->type != astNode::LEXEME
						|| !static_cast<LexemeNode*>(node.get())->lexeme.isLiteral(",")
						)
					{
						if (node->type == astNode::LEXEME
							&& static_cast<LexemeNode*>(node.get())->lexeme.isLiteral(")")
							)
						{
							break;
						}
						std::string err = "Parameter list expected righthand ',', found ";
						err.append(node->toString());
						SOUP_THROW(ParseError(std::move(err)));
					}

					node = ps.popRighthand();
					if (node->type != astNode::LEXEME
						|| static_cast<LexemeNode*>(node.get())->lexeme.token_keyword != Lexeme::LITERAL
						)
					{
						std::string err = "Parameter list expected righthand parameter after ',', found ";
						err.append(node->toString());
						SOUP_THROW(ParseError(std::move(err)));
					}
				}
			}

			node = ps.popRighthand();
			if (node->type != astNode::BLOCK)
			{
				std::string err = "'function' expected righthand block, found ";
				err.append(node->toString());
				SOUP_THROW(ParseError(std::move(err)));
			}

			static_cast<astBlock*>(node.get())->param_literals = std::move(param_literals);
			if (var_name_literal)
			{
				ps.setOp(OP_ASSIGN);
				ps.pushArgNode(std::move(var_name_literal));
				ps.pushArg(static_cast<astBlock*>(node.release()));
			}
			else
			{
				ps.pushLefthand(static_cast<astBlock*>(node.release()));
			}
		});
		ld.addToken("[", [](ParserState& ps)
		{
			ps.setOp(OP_INDEX);
			ps.consumeLefthandValue();
			ps.consumeRighthandValue();

			if (auto node = ps.popRighthand();
				node->type != astNode::LEXEME
				|| !static_cast<LexemeNode*>(node.get())->lexeme.isLiteral("]")
				)
			{
				std::string err = "'[' expected ']' after index/key, found ";
				err.append(node->toString());
				SOUP_THROW(ParseError(std::move(err)));
			}
		});
		ld.addToken(".", Rgb::RED, [](ParserState& ps)
		{
			ps.setOp(OP_CONCAT);
			ps.consumeLefthandValue();
			ps.consumeRighthandValue();
		});
		ld.addToken("==", Rgb::RED, [](ParserState& ps)
			{
				ps.setOp(OP_EQ);
				ps.consumeLefthandValue();
				ps.consumeRighthandValue();
			});
		ld.addToken("=", Rgb::RED, [](ParserState& ps)
		{
			ps.setOp(OP_ASSIGN);
			ps.consumeLefthandValue();
			ps.consumeRighthandValue();
		});
		ld.addToken("if", Rgb::RED, [](ParserState& ps)
		{
			ps.setOp(OP_IF);
			auto node = ps.popRighthand();
			if (node->type != astNode::LEXEME
				|| static_cast<LexemeNode*>(node.get())->lexeme.token_keyword != "("
				)
			{
				std::string err = "'if' expected righthand '(', found ";
				err.append(node->toString());
				SOUP_THROW(ParseError(std::move(err)));
			}

			ps.consumeRighthandValue(); // condition

			node = ps.popRighthand();
			if (node->type != astNode::LEXEME
				|| !static_cast<LexemeNode*>(node.get())->lexeme.isLiteral(")")
				)
			{
				std::string err = "'if(cond' expected righthand ')', found ";
				err.append(node->toString());
				SOUP_THROW(ParseError(std::move(err)));
			}

			node = ps.popRighthand();
			if (node->type != astNode::BLOCK
				&& (
					node->type != astNode::LEXEME
					|| !static_cast<LexemeNode*>(node.get())->lexeme.isLiteral(":")
					)
				)
			{
				std::string err = "'if(cond)' expected righthand block or ':', found ";
				err.append(node->toString());
				SOUP_THROW(ParseError(std::move(err)));
			}
			if (node->type != astNode::BLOCK)
			{
				node = ps.collapseRighthandBlock("endif");
			}
			if (node->type != astNode::BLOCK)
			{
				std::string err = "'if(cond)' expected righthand block, found ";
				err.append(node->toString());
				SOUP_THROW(ParseError(std::move(err)));
			}
			ps.pushArg(static_cast<astBlock*>(node.release()));
		});
		ld.addToken("endif");
		ld.addToken("else", Rgb::RED, [](ParserState& ps)
		{
			auto node = ps.popLefthand();
			if (node->type != astNode::OP
				|| static_cast<OpNode*>(node.get())->op.type != OP_IF
				)
			{
				std::string err = "'else' expected lefthand OP_IF, found ";
				err.append(node->toString());
				SOUP_THROW(ParseError(std::move(err)));
			}
			ps.setOp(OP_IF_ELSE);
			ps.setArgs(std::move(static_cast<OpNode*>(node.get())->op.args));

			node = ps.popRighthand();
			if (node->type != astNode::BLOCK)
			{
				std::string err = "'else' expected righthand block, found ";
				err.append(node->toString());
				SOUP_THROW(ParseError(std::move(err)));
			}
			ps.pushArg(static_cast<astBlock*>(node.release()));
		});
		ld.addToken("(", [](ParserState& ps)
		{
			ps.setOp(OP_CALL);
			ps.consumeLefthandValue();

			std::vector<UniquePtr<astNode>> arg_nodes{};

			auto node = ps.popRighthand();
			if (node->type != astNode::LEXEME
				|| !static_cast<LexemeNode*>(node.get())->lexeme.isLiteral(")")
				)
			{
				while (true)
				{
					if (!node->isValue())
					{
						std::string err = "Argument list expected value, found ";
						err.append(node->toString());
						SOUP_THROW(ParseError(std::move(err)));
					}
					arg_nodes.emplace_back(std::move(node));

					node = ps.popRighthand();
					if (node->type != astNode::LEXEME
						|| !static_cast<LexemeNode*>(node.get())->lexeme.isLiteral(",")
						)
					{
						if (node->type == astNode::LEXEME
							&& static_cast<LexemeNode*>(node.get())->lexeme.isLiteral(")")
							)
						{
							break;
						}
						std::string err = "Argument list expected righthand ',', found ";
						err.append(node->toString());
						SOUP_THROW(ParseError(std::move(err)));
					}

					node = ps.popRighthand();
				}
			}
			
			ps.pushArg((uint64_t)arg_nodes.size());
			for (auto& node : arg_nodes)
			{
				ps.pushArgNode(std::move(node));
			}
		});
		ld.addToken("require", Rgb::RED, [](ParserState& ps)
		{
			ps.setOp(OP_REQUIRE);
			ps.consumeRighthandValue();
		});
		ld.addToken("echo", Rgb::BLUE, [](ParserState& ps)
		{
			ps.setOp(OP_ECHO);
			ps.consumeRighthandValue();
		});
		return ld;
	}

	const LangDesc& PhpState::getLangDesc()
	{
		static LangDesc ld = getLangDescImpl();
		return ld;
	}

	static void processNonPhpmodeBuffer(std::vector<Lexeme>& ls, std::vector<Lexeme>::iterator& i, std::string& non_phpmode_buffer)
	{
		if (!non_phpmode_buffer.empty())
		{
			i = ls.insert(i, Lexeme{ Lexeme::VAL, std::move(non_phpmode_buffer) });
			i = ls.insert(i, Lexeme{ "echo" });
			non_phpmode_buffer.clear();
		}
	}

	void PhpState::processPhpmode(std::vector<Lexeme>& ls)
	{
		std::string non_phpmode_buffer{};
		auto i = ls.begin();
		for (; i != ls.end(); )
		{
			if (i->token_keyword == "<?php")
			{
				i = ls.erase(i);
				processNonPhpmodeBuffer(ls, i, non_phpmode_buffer);
				for (; i != ls.end(); )
				{
					if (i->token_keyword == "?>")
					{
						i = ls.erase(i);
						break;
					}
					++i;
				}
			}
			else
			{
				non_phpmode_buffer.append(i->getSourceString());
				i = ls.erase(i);
			}
		}
		processNonPhpmodeBuffer(ls, i, non_phpmode_buffer);
	}
#ifdef __clang__
#pragma clang diagnostic pop
#endif

	std::string PhpState::evaluate(const std::string& code, unsigned int max_require_depth) const
	{
		std::string output{};
		// Ideally, the parser would report errors in a less fatal way...
		SOUP_TRY
		{
			auto ld = getLangDesc();
			auto ls = ld.tokenise(code);
			processPhpmode(ls);
			ld.eraseNlTerminatedComments(ls, "//");
			ld.eraseSpace(ls);
			auto b = ld.parseImpl(ls);

			StringWriter w;
			b.compile(w);

			StringReader r{ std::move(w.data) };
			execute(output, r, max_require_depth);
		}
		SOUP_CATCH (std::runtime_error, e)
		{
			if (!output.empty())
			{
				output.push_back('\n');
			}
			output.append("ERROR: ");
			output.append(e.what());
		}
		return output;
	}

	struct CapturePhpVm
	{
		const PhpState& state;
		std::string& output;
		unsigned int max_require_depth;
	};

	void PhpState::execute(std::string& output, Reader& r, unsigned int max_require_depth, std::stack<SharedPtr<Mixed>>&& stack) const
	{
		LangVm vm(r, std::move(stack));
		vm.cap = CapturePhpVm{ *this, output, max_require_depth };

		// Setup variables
		std::unordered_map<Mixed, SharedPtr<Mixed>> _SERVER{};
		_SERVER.emplace("REQUEST_URI", soup::make_shared<Mixed>(request_uri));

		vm.vars.emplace("$_SERVER", soup::make_shared<Mixed>(std::move(_SERVER)));

		// Setup opcodes
		vm.addOpcode(OP_CONCAT, [](LangVm& vm)
		{
			std::string str = vm.pop()->toString();
			str.append(vm.pop()->toString());
			vm.push(std::move(str));
		});
		vm.addOpcode(OP_INDEX, [](LangVm& vm)
		{
			auto arr = vm.pop();
			auto key = vm.pop();
			if (auto e = arr->getMixedSpMixedMap().find(*key); e != arr->getMixedSpMixedMap().end())
			{
				vm.push(e->second);
			}
			else
			{
				std::string err = "Array has no entry with key ";
				err.append(key->toString());
				SOUP_THROW(ParseError(std::move(err)));
			}
		});
		vm.addOpcode(OP_ASSIGN, [](LangVm& vm)
		{
			auto& var = vm.popVarRef();
			var = vm.pop();
		});
		vm.addOpcode(OP_EQ, [](LangVm& vm)
		{
			vm.push(vm.pop()->toString() == vm.pop()->toString());
		});
		vm.addOpcode(OP_IF, [](LangVm& vm)
		{
			auto cond_val = vm.pop()->getInt();
			auto true_sr = vm.popFunc();
			if (cond_val)
			{
				auto& cap = vm.cap.get<CapturePhpVm>();
				cap.state.execute(cap.output, true_sr, cap.max_require_depth);
			}
		});
		vm.addOpcode(OP_IF_ELSE, [](LangVm& vm)
		{
			auto cond_val = vm.pop()->getInt();
			auto true_sr = vm.popFunc();
			auto false_sr = vm.popFunc();
			auto& cap = vm.cap.get<CapturePhpVm>();
			if (cond_val)
			{
				cap.state.execute(cap.output, true_sr, cap.max_require_depth);
			}
			else
			{
				cap.state.execute(cap.output, false_sr, cap.max_require_depth);
			}
		});
		vm.addOpcode(OP_CALL, [](LangVm& vm)
		{
			auto sr = vm.popFunc();
			auto num_args = vm.popRaw()->getUInt();
			std::stack<SharedPtr<Mixed>> handover_stack{};
			while (num_args--)
			{
				handover_stack.emplace(vm.pop());
			}
			auto& cap = vm.cap.get<CapturePhpVm>();
			cap.state.execute(cap.output, sr, cap.max_require_depth, std::move(handover_stack));
		});
		vm.addOpcode(OP_REQUIRE, [](LangVm& vm)
		{
			auto& cap = vm.cap.get<CapturePhpVm>();
			if (cap.max_require_depth == 0)
			{
				SOUP_THROW(std::runtime_error("Max require depth exceeded"));
			}
			std::filesystem::path file = cap.state.cwd;
			file /= vm.popString();
			if (!std::filesystem::exists(file))
			{
				std::string err = "Required file doesn't exist: ";
				err.append(file.string());
				SOUP_THROW(std::runtime_error(std::move(err)));
			}
			cap.output.append(cap.state.evaluate(string::fromFile(file.string()), cap.max_require_depth - 1));
		});
		vm.addOpcode(OP_ECHO, [](LangVm& vm)
		{
			auto& cap = vm.cap.get<CapturePhpVm>();
			cap.output.append(vm.pop()->toString());
		});

		// Let's do this
		vm.execute();
	}
}

#include "cli.hpp"

#include <iostream>
#include <thread>

#include <audDevice.hpp>
#include <audPlayback.hpp>
#include <Canvas.hpp>
#include <Chatbot.hpp>
#include <ChessCli.hpp>
#include <CompiledExecutable.hpp>
#include <country_names.hpp>
#include <console.hpp>
#include <dhcp.hpp>
#include <Editor.hpp>
#include <FileReader.hpp>
#include <HttpRequest.hpp>
#include <hwGamepad.hpp>
#include <hwHid.hpp>
#include <netIntel.hpp>
#include <os.hpp>
#include <QrCode.hpp>
#include <riff.hpp>
#include <string.hpp>
#include <unicode.hpp>
#include <Uri.hpp>
#include <wasm.hpp>
#include <WavFmtChunk.hpp>

using namespace soup;

int main(int argc, const char** argv)
{
	if (argc > 1)
	{
		std::string subcommand = argv[1];
		string::lower(subcommand);

		if (subcommand == "3d")
		{
			cli_3d();
			return 0;
		}

		if (subcommand == "bench")
		{
			cli_bench();
			return 0;
		}

		if (subcommand == "cat2json")
		{
			return cli_cat2json(argc, argv);
		}

		if (subcommand == "chatbot")
		{
			while (true)
			{
				console.init(false);
				std::cout << "> ";
				std::string line;
				if (!std::getline(std::cin, line))
				{
					break;
				}
				try
				{
					auto res = Chatbot::process(line);
					std::cout << res.response << std::endl;
					if (res.type == CB_RES_IMAGE)
					{
						std::cout << res.extra.getCanvas().toStringDownsampledDoublewidthUtf8(true, true);
					}
				}
				catch (std::exception& e)
				{
					std::cout << "Error: " << e.what() << std::endl;
				}
			}
			return 0;
		}

		if (subcommand == "chatgpt")
		{
			if (argc > 2)
			{
				cli_chatgpt(argc - 2, &argv[2]);
			}
			else
			{
				std::cout << "Syntax: soup chatgpt [token] <model>" << std::endl;
			}
			return 0;
		}

		if (subcommand == "chess")
		{
			ChessCli cc{};
			if (argc > 2)
			{
				cc.board.loadFen(argv[2]);
			}
			cc.run();
			return 0;
		}

		if (subcommand == "datareflection")
		{
			cli_datareflection();
			return 0;
		}

		if (subcommand == "dhcp")
		{
			if (auto info = dhcp::requestInfo(); info.isValid())
			{
				std::cout << "Router \"" << info.server_name << "\" responded to DHCPInform and provided DNS server " << IpAddr(info.dns_server).toString() << "\n";
			}
			else
			{
				std::cout << "I screamed into the void, but the void did not respond. Possibly VPN network?\n";
			}
			return 0;
		}

		if (subcommand == "dig")
		{
			cli_dig(argc - 2, &argv[2]);
			return 0;
		}

		if (subcommand == "dnsserver")
		{
			if (argc > 2)
			{
				cli_dnsserver(argc - 2, &argv[2]);
			}
			else
			{
				std::cout << "Syntax: soup dnsserver [file] <bind-ip>" << std::endl;
			}
			return 0;
		}

		if (subcommand == "dvd")
		{
			cli_dvd();
			return 0;
		}

		if (subcommand == "edit")
		{
			if (argc <= 2)
			{
				std::cout << "Syntax: soup edit [files...]" << std::endl;
				return 0;
			}
			Editor edit{};
			for (int i = 2; i != argc; ++i)
			{
				auto& tab = edit.addTab(argv[i], unicode::utf8_to_utf32(string::fromFile(argv[i])));
				if (i == 2)
				{
					tab.setActive(edit);
				}
			}
			edit.run();
			return 0;
		}

		if (subcommand == "gamepad")
		{
			for (auto& gp : hwGamepad::getAll())
			{
				console.init(false);
				const std::string name = gp.name ? gp.name : gp.hid.getProductName();
				std::cout << name << " detected, awaiting input.\n";
				hwGamepad::Status prev_status{};
				while (true)
				{
					auto status = gp.receiveStatus();
					if (gp.disconnected)
					{
						std::cout << name << " disconnected.\n";
						return 0;
					}
					if (memcmp(&status, &prev_status, sizeof(hwGamepad::Status)) != 0)
					{
						prev_status = status;
						console.clearScreen();
						console.setCursorPos(0, 0);
						std::cout << name << "\n";
						std::cout << "Left Stick: " << status.left_stick_x << ", " << status.left_stick_y << "\n";
						std::cout << "Right Stick: " << status.right_stick_x << ", " << status.right_stick_y << "\n";
						if (gp.hasAnalogueTriggers())
						{
							std::cout << "Left Trigger: " << status.left_trigger << "\n";
							std::cout << "Right Trigger: " << status.right_trigger << "\n";
						}
						std::cout << "Buttons: ";
						std::vector<std::string> buttons{};
						if (status.buttons[BTN_DPAD_UP]) buttons.emplace_back("DPAD UP");
						if (status.buttons[BTN_DPAD_DOWN]) buttons.emplace_back("DPAD DOWN");
						if (status.buttons[BTN_DPAD_LEFT]) buttons.emplace_back("DPAD LEFT");
						if (status.buttons[BTN_DPAD_RIGHT]) buttons.emplace_back("DPAD RIGHT");
						if (status.buttons[BTN_ACT_DOWN]) buttons.emplace_back("ACT DOWN");
						if (status.buttons[BTN_ACT_RIGHT]) buttons.emplace_back("ACT RIGHT");
						if (status.buttons[BTN_ACT_LEFT]) buttons.emplace_back("ACT LEFT");
						if (status.buttons[BTN_ACT_UP]) buttons.emplace_back("ACT UP");
						if (status.buttons[BTN_LBUMPER]) buttons.emplace_back("L1");
						if (status.buttons[BTN_RBUMPER]) buttons.emplace_back("R1");
						if (status.buttons[BTN_LTRIGGER]) buttons.emplace_back("L2");
						if (status.buttons[BTN_RTRIGGER]) buttons.emplace_back("R2");
						if (status.buttons[BTN_LSTICK]) buttons.emplace_back("L3");
						if (status.buttons[BTN_RSTICK]) buttons.emplace_back("R3");
						if (status.buttons[BTN_META]) buttons.emplace_back("META");
						if (status.buttons[BTN_TOUCHPAD]) buttons.emplace_back("TOUCHPAD");
						if (status.buttons[BTN_SHARE]) buttons.emplace_back("SHARE");
						if (status.buttons[BTN_OPTIONS]) buttons.emplace_back("OPTIONS");
						if (status.buttons[BTN_MINUS]) buttons.emplace_back("MINUS");
						if (status.buttons[BTN_PLUS]) buttons.emplace_back("PLUS");
						if (status.buttons[BTN_ASSISTANT]) buttons.emplace_back("ASSISTANT");
						if (status.buttons[BTN_MENU]) buttons.emplace_back("MENU");
						if (buttons.empty())
						{
							std::cout << "None";
						}
						else
						{
							std::cout << string::join(buttons, ", ");
						}
						std::cout << "\n";
						if (gp.hasTouchpad())
						{
							std::cout << "Touchpad: ";
							std::vector<std::string> finger_coords{};
							for (uint8_t i = 0; i != status.num_fingers_on_touchpad; ++i)
							{
								std::string str = std::to_string(status.finger_coords[i].x);
								str.append(", ");
								str.append(std::to_string(status.finger_coords[i].y));
								finger_coords.emplace_back(std::move(str));
							}
							if (finger_coords.empty())
							{
								std::cout << "Inactive";
							}
							else
							{
								std::cout << string::join(finger_coords, "; ");
							}
							std::cout << "\n";
						}
						if (gp.canRumble())
						{
							std::cout << "Press DPAD UP+LEFT to rumble: Left trigger to actuate weak motor, right trigger to actuate strong motor.\n";
							if (status.buttons[BTN_DPAD_UP] && status.buttons[BTN_DPAD_LEFT])
							{
								gp.rumbleWeak(static_cast<uint8_t>(status.left_trigger * 255), 100);
								gp.rumbleStrong(static_cast<uint8_t>(status.right_trigger * 255), 100);
							}
						}
						if (gp.hasLight())
						{
							std::cout << "Press DPAD DOWN+RIGHT to change the light: Use right trigger to adjust hue and left trigger to adjust value.\n";
							if (status.buttons[BTN_DPAD_DOWN] && status.buttons[BTN_DPAD_RIGHT])
							{
								gp.setLight(Rgb::fromHsv(status.right_trigger * 360.0, 1.0, /*1.0 -*/ status.left_trigger));
							}
						}
						std::this_thread::sleep_for(std::chrono::milliseconds(7));
					}
				}
				break;
			}
			std::cout << "No gamepad detected.\n";
			return 0;
		}

		if (subcommand == "geoip")
		{
			IpAddr addr;
			if (argc != 3
				|| !addr.fromString(argv[2])
				)
			{
				std::cout << "Syntax: soup geoip [ip]" << std::endl;
				return 0;
			}
			netIntel intel;
			if (addr.isV4())
			{
				std::cout << "Initialising netIntel for IPv4..." << std::endl;
				intel.init(true, false);
			}
			else
			{
				std::cout << "Initialising netIntel for IPv6..." << std::endl;
				intel.init(false, true);
			}
			if (auto loc = intel.getLocationByIp(addr))
			{
				if (loc->city)
				{
					std::cout << "City: " << loc->city << std::endl;
				}
				if (loc->state)
				{
					std::cout << "State: " << loc->state << std::endl;
				}
				if (auto cn = getCountryName(loc->country_code.c_str()))
				{
					std::cout << "Country: " << cn << std::endl;
				}
				else
				{
					std::cout << "Country: " << loc->country_code.c_str() << std::endl;
				}
			}
			if (auto as = intel.getAsByIp(addr))
			{
				std::cout << "AS Number: " << as->number << "\n";
				std::cout << "AS Handle: " << as->handle << "\n";
				std::cout << "AS Name: " << as->name << "\n";
				std::cout << "AS is hosting provider? " << (as->isHosting(intel) ? "Yes" : "No") << "\n";
			}
			return 0;
		}

		if (subcommand == "hid")
		{
			std::cout << "VID\tPID\n";
			for (const auto& hid : hwHid::getAll())
			{
				std::cout << std::hex;
				std::cout << hid.vendor_id << "\t" << hid.product_id << "\t";
				if (auto name = hid.getProductName(); !name.empty())
				{
					std::cout << name;
				}
				else
				{
					if (auto vendor = hid.getManufacturerName(); !vendor.empty())
					{
						std::cout << vendor << " Device ";
					}
					std::cout << "[Product Name Not Provided]";
				}
				std::cout << "\n";
				std::cout << "\t\t- Path: " << hid.path << "\n";
				std::cout << "\t\t- Usage Page: " << hid.usage_page << "\n";
				std::cout << "\t\t- Usage: " << hid.usage << "\n";
				std::cout << "\t\t- Bus: " << (hid.isBluetooth() ? "BT" : "USB") << "\n";
				std::cout << std::dec;
				std::cout << "\t\t- Input Report Byte Length: " << hid.input_report_byte_length << "\n";
				std::cout << "\t\t- Output Report Byte Length: " << hid.output_report_byte_length << "\n";
				std::cout << "\t\t- Feature Report Byte Length: " << hid.feature_report_byte_length << "\n";
#if !SOUP_WINDOWS
				std::cout << "\t\t- Have Permission? " << (hid.havePermission() ? "Yes" : "No") << "\n";
#endif
				if (auto serial = hid.getSerialNumber(); !serial.empty())
				{
					std::cout << "\t\t- Serial: " << serial << "\n";
				}
			}
			return 0;
		}

		if (subcommand == "html")
		{
			if (argc != 3)
			{
				std::cout << "Syntax: soup html [file]" << std::endl;
				return 0;
			}
			cli_html(argv[2]);
			return 0;
		}

		if (subcommand == "http")
		{
			if (argc != 3)
			{
				std::cout << "Syntax: soup http [uri]" << std::endl;
				return 0;
			}
			auto hr = HttpRequest(Uri(argv[2]));
			auto res = hr.execute();
			if (res.has_value())
			{
				console.init(false); // to "correctly" display UTF-8 on Windows
				std::cout << res->status_code << "\n";
				std::cout << res->toString() << "\n";
			}
			else
			{
				std::cout << "Request failed.\n";
			}
			return 0;
		}

		if (subcommand == "ircserver")
		{
			return cli_ircserver();
		}

		if (subcommand == "keyboard")
		{
			cli_keyboard();
			return 0;
		}

		if (subcommand == "maze")
		{
			cli_maze();
			return 0;
		}

		if (subcommand == "mesh")
		{
			return cli_mesh(argc - 2, &argv[2]);
		}

		if (subcommand == "midi")
		{
			cli_midi(argc - 2, &argv[2]);
			return 0;
		}

		if (subcommand == "morse")
		{
			cli_morse(argc - 2, &argv[2]);
			return 0;
		}

		if (subcommand == "mouse")
		{
			cli_mouse();
			return 0;
		}

		if (subcommand == "qr")
		{
			if (argc != 3)
			{
				std::cout << "Syntax: soup qr [contents]" << std::endl;
				return 0;
			}
			auto qrcode = QrCode::encodeText(argv[2]);
			console.init(false);
			console << qrcode.toCanvas(4, true).toStringDownsampledDoublewidth(true, true);
			console.resetColour();
			return 0;
		}

		if (subcommand == "repl")
		{
			cli_repl();
			return 0;
		}

		if (subcommand == "script")
		{
			if (argc != 3)
			{
				std::cout << "Syntax: soup script [.cpp file]" << std::endl;
				return 0;
			}
			auto res = CompiledExecutable::fromCpp(argv[2]);
			std::cout << res.compiler_output;
			if (res.exe_file.exists())
			{
				std::cout << os::execute(res.exe_file);
			}
			return 0;
		}

		if (subcommand == "snake")
		{
			cli_snake();
			return 0;
		}

		if (subcommand == "test")
		{
			cli_test();
			return 0;
		}

		if (subcommand == "wasm")
		{
			FileReader fr(argv[2]);
			WasmScript scr;
			if (!scr.load(fr))
			{
				std::cout << "Failed to load\n";
				return 1;
			}
			auto code = scr.getExportedFuntion("_start");
			if (!code)
			{
				std::cout << "WASM file has loaded but \"_start\" function not found in exports.\n";
				return 2;
			}
			scr.linkWasiPreview1();
			WasmVm vm(scr);
			if (!vm.run(*code))
			{
				std::cout << "A runtime error occurred.\n";
				return 3;
			}
			return 0;
		}

		if (subcommand == "wav")
		{
#if SOUP_WINDOWS || SOUP_LINUX
			if (argc != 3)
			{
				std::cout << "Syntax: soup wav [file]" << std::endl;
				return 0;
			}
			static FileReader fr(argv[2]);
			RiffReader rr(fr);
			if (rr.seekChunk("fmt ").isValid())
			{
				WavFmtChunk fmt;
				fmt.read(fr);
				if (fmt.isGoodForAudPlayback()
					&& rr.seekChunk("data").isValid()
					)
				{
					auto dev = audDevice::getDefault();
					std::cout << "WAV is good, playing on " << dev.getName() << "\n";
					auto pb = dev.open(fmt.channels, [](audPlayback& pb, audSample* block)
					{
						if (!fr.hasMore())
						{
							pb.stop();
							return;
						}
						std::string data;
						fr.str(AUD_BLOCK_BYTES, data);
						memcpy(block, data.data(), data.size());
					});
					pb->awaitCompletion();
					return 0;
				}
			}
			std::cout << "WAV is bad.\n";
#else
			std::cout << "Sorry, audio is currently not supported on your platform.\n";
#endif
			return 0;
		}

		if (subcommand == "websrv")
		{
			if (argc != 3)
			{
				std::cout << "Syntax: soup websrv [dir]" << std::endl;
				return 0;
			}
			return cli_websrv(argv[2]);
		}
	}

	std::cout << R"EOC(Syntax: soup [tool]

Available tools: 3d, bench, cat2json, chatbot, chatgpt, chess, datareflection, dhcp, dig, dnsserver, dvd, edit, gamepad, geoip, hid, html, http, ircserver, keyboard, maze, mesh, midi, morse, mouse, qr, repl, script, snake, test, wasm, wav, websrv

Legend: [Required] <Optional>)EOC" << std::endl;
	return 0;
}

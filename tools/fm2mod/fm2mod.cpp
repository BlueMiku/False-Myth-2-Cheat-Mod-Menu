// fm2mod - FalseMyth2 data modding helper built on the (MIT) WolfTL WolfRPG headers.
// Build: see build.ps1.  Usage: fm2mod dump-triggers <CommonEvent.dat>
#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <windows.h>
// WolfRPG headers keep the interesting fields private; we want read/write access.
#define private public
#define protected public
#include "WolfRPG/WolfRPG.hpp"
#undef private
#undef protected
#include "evbuilder.hpp"

static std::string hex(const std::vector<uint8_t>& b)
{
	std::string s;
	for (uint8_t x : b) s += std::format("{:02X} ", x);
	return s;
}

int main(int argc, char** argv)
{
	SetConsoleOutputCP(CP_UTF8);
	if (argc < 3) { std::cerr << "usage: fm2mod dump-triggers <CommonEvent.dat>\n"; return 1; }
	std::string mode = argv[1];
	try
	{
		if (mode == "dump-triggers")
		{
			std::filesystem::path cep = argv[2]; ::CommonEvents ce{cep};
			for (const CommonEvent& ev : ce.m_events)
			{
				std::cout << ev.m_intId << "\t" << ToUTF8(ev.m_name) << "\tcmds=" << ev.m_commands.size()
						  << "\tu1=" << ev.m_unknown1 << "\tu2=" << hex(ev.m_unknown2) << "\tu7=" << hex(ev.m_unknown7)
						  << "\tu9=" << ToUTF8(ev.m_unknown9) << "\tu3n=" << ev.m_unknown3.size() << "\tu10v=" << ev.m_unknown10Valid
						  << "\n";
			}
			return 0;
		}
		if (mode == "route-stats" && argc >= 3)
		{
			// histogram of movement-route command ids/args used by Move (201) commands in common events
			std::filesystem::path cep = argv[2]; ::CommonEvents ce{cep};
			std::map<std::string, int> hist; std::map<std::string, std::string> sample;
			for (auto& ev : ce.m_events)
				for (size_t k = 0; k < ev.m_commands.size(); k++)
				{
					auto mv = std::dynamic_pointer_cast<Command::CommandSpecialClasses::Move>(ev.m_commands[k]);
					if (!mv) continue;
					for (auto& rc : mv->m_route)
					{
						std::string key = std::to_string((int)rc.m_id) + "(";
						for (auto a : rc.m_args) key += std::to_string((int)a) + ",";
						key += ")";
						hist[key]++;
						if (!sample.count(key)) sample[key] = std::to_string(ev.m_intId) + ":" + ToUTF8(ev.m_name) + "@" + std::to_string(k);
					}
				}
			for (auto& [k, n] : hist) std::cout << k << "\t" << n << "\t" << sample[k] << std::endl;
			return 0;
		}
		if (mode == "route-event" && argc >= 4)
		{
			// every Move command of one event: index, target, flags, route
			std::filesystem::path cep = argv[2]; ::CommonEvents ce{cep};
			auto& ev = ce.m_events.at(std::stoi(argv[3]));
			std::cout << "event " << ev.m_intId << " " << ToUTF8(ev.m_name) << std::endl;
			for (size_t k = 0; k < ev.m_commands.size(); k++)
			{
				auto mv = std::dynamic_pointer_cast<Command::CommandSpecialClasses::Move>(ev.m_commands[k]);
				if (!mv) continue;
				std::cout << "  @" << k << " target=" << (int32_t)mv->m_args[0] << " flags=" << (int)mv->m_flags << " route=";
				for (auto& rc : mv->m_route) { std::cout << (int)rc.m_id << "("; for (auto a : rc.m_args) std::cout << (int32_t)a << ","; std::cout << ") "; }
				std::cout << std::endl;
			}
			return 0;
		}
		if (mode == "route-speed" && argc >= 3)
		{
			// every Move command whose route sets the move speed (route id 29): event, command index, target, flags, whole route
			std::filesystem::path cep = argv[2]; ::CommonEvents ce{cep};
			for (auto& ev : ce.m_events)
				for (size_t k = 0; k < ev.m_commands.size(); k++)
				{
					auto mv = std::dynamic_pointer_cast<Command::CommandSpecialClasses::Move>(ev.m_commands[k]);
					if (!mv) continue;
					bool has = false;
					for (auto& rc : mv->m_route) if (rc.m_id == 29) has = true;
					if (!has) continue;
					std::cout << ev.m_intId << ":" << ToUTF8(ev.m_name) << "@" << k << " target=" << (int32_t)mv->m_args[0] << " flags=" << (int)mv->m_flags << " unk=";
					for (auto u : mv->m_unknown) std::cout << (int)u << ",";
					std::cout << " route=";
					for (auto& rc : mv->m_route) { std::cout << (int)rc.m_id << "("; for (auto a : rc.m_args) std::cout << (int32_t)a << ","; std::cout << ") "; }
					std::cout << std::endl;
				}
			return 0;
		}
		if (mode == "map-info" && argc >= 3)
		{
			std::filesystem::path mp = argv[2];
			Map m{mp, false};
			std::cout << "size " << m.m_width << "x" << m.m_height << " events " << m.m_events.size() << std::endl;
			for (auto& e : m.m_events) std::cout << "  ev " << e.m_id << " at (" << e.m_x << "," << e.m_y << ")" << std::endl;
			return 0;
		}
		if (mode == "show-event" && argc >= 4)
		{
			std::filesystem::path cep = argv[2]; ::CommonEvents ce{cep};
			int id = std::stoi(argv[3]); int maxn = argc >= 5 ? std::stoi(argv[4]) : 30;
			const CommonEvent& ev = ce.m_events.at(id);
			std::cout << "event " << id << " name=" << ToUTF8(ev.m_name) << " cmds=" << ev.m_commands.size() << std::endl;
			int n = 0;
			for (auto& c : ev.m_commands)
			{
				std::cout << n << " code=" << (int)c->m_cid << " indent=" << (int)c->m_indent << " args=[";
				for (auto a : c->m_args) std::cout << a << ",";
				std::cout << "] strs=" << c->m_stringArgs.size() << " v35u=" << c->m_v35Unknown.size() << std::endl;
				if (++n >= maxn) break;
			}
			return 0;
		}
		if (mode == "build-probe" && argc >= 5)
		{
			std::filesystem::path inp = argv[2], outp = argv[3];
			std::string variant = argv[4];
			::CommonEvents ce{inp};
			namespace C = Command;
			using CT = C::CommandType;
			auto blackPic = [&]() { return std::make_shared<C::CommandSpecialClasses::Picture>(CT::Picture,
				uInts{0, 999999, 0, 1, 1, 1, 255, 0, 0, 200, 0}, tStrings{L"Picture/まっくら.jpg"}, 0); };
			if (variant == "A") // prepend black picture to existing parallel event 119
			{
				auto& cmds = ce.m_events.at(119).m_commands;
				cmds.insert(cmds.begin(), blackPic());
			}
			else if (variant == "B") // new parallel event cloned from 119's header
			{
				CommonEvent ev = ce.m_events.at(119);
				ev.m_intId = (uint32_t)ce.m_events.size(); ev.m_id = ev.m_intId;
				ev.m_name = L"FM2 probe B"; ev.m_description = L"";
				ev.m_commands.clear(); ev.m_commands.push_back(blackPic()); ev.m_commands.push_back(std::make_shared<C::Command>(CT::Blank, uInts{}, tStrings{}, 0));
				ce.m_events.push_back(ev);
			}
			else if (variant == "C") // new call-only event + 119 calls it
			{
				CommonEvent ev = ce.m_events.at(0); // a call-only event as template
				ev.m_intId = (uint32_t)ce.m_events.size(); ev.m_id = ev.m_intId;
				ev.m_name = L"FM2 probe C"; ev.m_description = L"";
				ev.m_commands.clear(); ev.m_commands.push_back(blackPic()); ev.m_commands.push_back(std::make_shared<C::Command>(CT::Blank, uInts{}, tStrings{}, 0));
				uint32_t newId = ev.m_intId;
				ce.m_events.push_back(ev);
				auto& cmds = ce.m_events.at(119).m_commands;
				cmds.insert(cmds.begin(), std::make_shared<C::Command>(CT::CommonEvent, uInts{500000 + newId, 0}, tStrings{}, 0));
			}
			else if (variant == "C2") // reuse blank slot 933 as call-only event; 119 calls it
			{
				auto& ev = ce.m_events.at(933);
				ev.m_name = L"FM2 probe C2";
				ev.m_commands.clear(); ev.m_commands.push_back(blackPic()); ev.m_commands.push_back(std::make_shared<C::Command>(CT::Blank, uInts{}, tStrings{}, 0));
				auto& cmds = ce.m_events.at(119).m_commands;
				cmds.insert(cmds.begin(), std::make_shared<C::Command>(CT::CommonEvent, uInts{500000 + 933, 0}, tStrings{}, 0));
			}
			else if (variant == "D") // reuse blank slot 934 as a PARALLEL event (header copied from 119)
			{
				CommonEvent& src = ce.m_events.at(119);
				auto& ev = ce.m_events.at(934);
				ev.m_unknown1 = src.m_unknown1; ev.m_unknown2 = src.m_unknown2;
				ev.m_name = L"FM2 probe D";
				ev.m_commands.clear(); ev.m_commands.push_back(blackPic()); ev.m_commands.push_back(std::make_shared<C::Command>(CT::Blank, uInts{}, tStrings{}, 0));
			}
			else if (variant == "K") // key-code probe: parallel event in slot 934 shows the pressed key code
			{
				CommonEvent& src = ce.m_events.at(119);
				auto& ev = ce.m_events.at(934);
				ev.m_unknown1 = src.m_unknown1; ev.m_unknown2 = src.m_unknown2;
				ev.m_name = L"FM2 key probe";
				ev.m_commands.clear();
				ev.m_commands.push_back(std::make_shared<C::Command>(CT::InputKey, uInts{1600000, 256, 0}, tStrings{}, 0));
				ev.m_commands.push_back(std::make_shared<C::CommandSpecialClasses::Picture>(CT::Picture,
					uInts{32, 99999, 0, 1, 1, 1, 255, 0, 0, 150, 0}, tStrings{L"KEY=\\cself[0]"}, 0));
				ev.m_commands.push_back(std::make_shared<C::Command>(CT::Blank, uInts{}, tStrings{}, 0));
			}
			else if (variant == "K2") // specific-key probe for candidate codes (Wolf code = 100 + DirectInput scancode)
			{
				CommonEvent& src = ce.m_events.at(119);
				auto& ev = ce.m_events.at(934);
				ev.m_unknown1 = src.m_unknown1; ev.m_unknown2 = src.m_unknown2;
				ev.m_name = L"FM2 key probe 2";
				ev.m_commands.clear();
				struct K { uint32_t code; const wchar_t* nm; };
				K ks[] = { {310,L"INS"}, {311,L"DEL"}, {299,L"HOME"}, {307,L"END"}, {301,L"PGUP"}, {309,L"PGDN"}, {145,L"X"}, {128,L"ENT"}, {130,L"A"}, {101,L"ESC"} };
				std::wstring text;
				int i = 1;
				const int freeIdx[] = {9, 10, 13, 17, 20, 21, 22, 36, 43, 44}; // allocated+unreferenced vars of spare group 9
				for (auto& k : ks)
				{
					int idx = freeIdx[i - 1];
					ev.m_commands.push_back(std::make_shared<C::Command>(CT::InputKey, uInts{2900000u + (uint32_t)idx, 4352, k.code}, tStrings{}, 0));
					text += std::wstring(k.nm) + L"=\\v9[" + std::to_wstring(idx) + L"] ";
					if (i == 5) text += L"\n";
					i++;
				}
				ev.m_commands.push_back(std::make_shared<C::CommandSpecialClasses::Picture>(CT::Picture,
					uInts{32, 99999, 0, 1, 1, 1, 255, 0, 0, 100, 0}, tStrings{text}, 0));
				ev.m_commands.push_back(std::make_shared<C::Command>(CT::Blank, uInts{}, tStrings{}, 0));
			}
			else if (variant == "BP") // probe: does BASIC input (keyboard+pad unified) still read while BanInput is active? Insert opens, X closes.
			{
				using EB = FM2::EB;
				const uint32_t V9 = 2000009, M = 2800000, LI = 2800006;
				const uint32_t banMask = getenv("FM2_BAN") ? (uint32_t)std::stoul(getenv("FM2_BAN")) : 496u;
				CommonEvent& src = ce.m_events.at(119);
				{
					auto& ev = ce.m_events.at(500);
					ev.m_unknown1 = src.m_unknown1; ev.m_unknown2 = src.m_unknown2;
					ev.m_name = L"FM2 probe open";
					ev.m_commands.clear();
					EB b(ev.m_commands);
					b.edge(1600000, 310, LI, [&] { b.set(M, 1); b.set(V9, 0); b.cmd(CT::BanInput, {banMask}); });
				}
				{
					auto& ev = ce.m_events.at(934);
					ev.m_unknown1 = (M << 8) | 0x22; ev.m_unknown2 = src.m_unknown2; ev.m_unknown2[1] = 1;
					ev.m_name = L"FM2 probe basic";
					ev.m_commands.clear();
					EB b(ev.m_commands);
					b.cmd(CT::InputKey, {1600001, 49});                                   // basic: dir4 + ok + cancel (no wait)
					b.cmd(CT::InputKey, {1600002, 4352, 145});                            // specific X (keyboard)
					b.cmd(CT::InputKey, {1600003, 4352, 308});                            // specific Down (keyboard)
					b.pic({32, 99999, 0, 1, 1, 1, 255, 20, 100, 120, 0}, L"BASIC=\\cself[1]   X=\\cself[2]   DOWN=\\cself[3]   (ban mask shown by build)");
					b.ifEq(1600002, 145, [&] { b.set(M, 0); b.set(V9, 1); b.cmd(CT::BanInput, {247}); b.cmd(CT::Picture, {2, 99999, 0}); });
				}
			}
			else if (variant == "MENU1") // Insert opens a paged cheat menu; field input frozen (V9=0 + BanInput) while open
			{
				using EB = FM2::EB;
				const uint32_t V9 = 2000009, V10 = 2000010, V11 = 2000011, AP = 2000001, LINEL = 2000002, V22 = 2000022, V31 = 2000031;
				// free global variables (allocated in SysDatabase, never referenced by the game or used as trigger vars)
				const uint32_t M = 2800000, S = 2800005, LI = 2800006, LU = 2800007, LD = 2800008, LZ = 2800009, LX = 2900000, CL = 2900009;
				const uint32_t FZ = 2900010, PT = 2900013, PC = 2900017;              // time freeze flag, pinned slot, pinned counter
				const uint32_t PG = 2900021, WP = 2900022, PS = 2900036;              // current page, pending warp (map id), saved main-menu cursor
				const uint32_t T31 = 2900043, T22 = 2900044;                          // saved clock/counter around a warp
				const uint32_t LC = 2300029;                                          // latch for the controller combo
				const uint32_t PAD_L3 = 309, PAD_R3 = 310;                            // controller open/close combo: click both sticks (codes read with the pad probe; Select+Start is taken by Steam)
				const bool SPEED = (getenv("FM2_NO_SPEED") == nullptr);                 // walking-speed feature on/off at build time
				const int CUSTOM_PG = SPEED ? 7 : 6;                                    // index of the custom-amount page (after the optional speed page)
				const int WARP0 = CUSTOM_PG + 1;                                        // index of the first warp page
				const uint32_t LL = 2600004, LR = 2600009;                              // latches for Left / Right
				const uint32_t HC = 2900020;                                            // how many frames Left/Right has been held (auto-repeat on the amount row)
				const uint32_t CUR_CUSTOM_ROW = 8;                                      // row of "Custom amount" on the Currency page (Back returns the cursor there)
				const uint32_t CA = 2500000, STEP = 2700000;                            // custom amount (signed) and its step size per Left/Right press
				const uint32_t WF = 2600001, WC = 2600002, WD = 2600003;               // arrival watchdog: armed flag, frame counter; debug-overlay enable
				const uint32_t SE = 2400016, SS = 2400017;                            // hero walking-speed pin: enable flag, chosen speed
				const uint32_t WPF = 2300026, ZE = 2300027, ZV = 2300028;           // warp-event trigger flag, zoom-pin enable, pinned zoom value
				const uint32_t C0 = 1600000, C1 = 1600001, C2 = 1600002, C3 = 1600003, C4 = 1600004; // event-local temporaries (only 0-4 exist)
				const uint32_t KEY_INS = 310;
				const uint32_t BASIC_DIR4_OK_CANCEL = 0x31;                           // basic input: 4 directions + confirm + cancel, no waiting
				struct WT { uint32_t id, x, y, opt; const wchar_t* name; };
				const WT targets[] = {
#include "warp_targets.inc"
				};
				const int NT = (int)(sizeof(targets) / sizeof(targets[0]));
				const int PER = 11;                                                   // warp targets per page
				const int WARP_PAGES = (NT + PER - 1) / PER;
				struct EQI { uint32_t id; int type; const wchar_t* name; };
				const EQI equip[] = {
#include "equipment_items.inc"
				};
				const int NE = (int)(sizeof(equip) / sizeof(equip[0]));
				const int EQ_PAGES = (NE + PER - 1) / PER;
				const uint32_t perkIds[] = {
#include "shop_perks.inc"
				};
				const int NPERK = (int)(sizeof(perkIds) / sizeof(perkIds[0]));
				const uint32_t TL = 2100005;                                          // love-level editor: target level (0-10)
				// --- equipment navigation layout: all items / by type / by first letter. Page numbers are computed up front so rows can jump to pages built later.
				struct CatDef { const wchar_t* name; std::vector<int> types; };
				const std::vector<CatDef> cats = {                                     // UserDB 15 field 2 ("equipment id type") grouped into player-facing categories
					{L"Swords", {1}}, {L"Spears", {2}}, {L"Axes & hammers", {3}}, {L"Armor & robes", {4}}, {L"Bows", {5}},
					{L"Staves", {9}}, {L"Shields", {10}}, {L"Gauntlets & claws", {11}}, {L"Charms & consumables", {8}}, {L"Gems", {7, 12}} };
				std::vector<std::vector<int>> catItems(cats.size());                   // indices into equip[]
				std::map<wchar_t, std::vector<int>> letterMap;
				for (int i = 0; i < NE; i++)
				{
					for (size_t c = 0; c < cats.size(); c++)
						for (int t : cats[c].types) if (t == equip[i].type) catItems[c].push_back(i);
					wchar_t ch = equip[i].name[0];
					if (ch >= L'a' && ch <= L'z') ch = (wchar_t)(ch - 32);
					letterMap[(ch >= L'A' && ch <= L'Z') ? ch : L'#'].push_back(i);
				}
				std::vector<wchar_t> letters;
				for (auto& kv : letterMap) letters.push_back(kv.first);
				if (!letters.empty() && letters.front() == L'#') { letters.erase(letters.begin()); letters.push_back(L'#'); }   // '#' (names not starting with a letter) goes last
				auto pagesFor = [&](size_t n) { return (int)((n + PER - 1) / PER); };
				const int EQ0 = WARP0 + WARP_PAGES;                                   // all-items pages
				const int EQHUB = EQ0 + EQ_PAGES;                                     // "Give equipment" hub
				const int TYPELIST = EQHUB + 1;                                       // list of categories
				std::vector<int> catFirst(cats.size()), catPages(cats.size());
				int cur = TYPELIST + 1;
				for (size_t c = 0; c < cats.size(); c++) { catFirst[c] = cur; catPages[c] = pagesFor(catItems[c].size()); cur += catPages[c]; }
				const int LPS = 13;                                                   // letters per letter-select page
				const int LS0 = cur;                                                  // letter-select pages
				const int LSN = ((int)letters.size() + LPS - 1) / LPS;
				cur += LSN;
				std::vector<int> letFirst(letters.size()), letPages(letters.size());
				for (size_t j = 0; j < letters.size(); j++) { letFirst[j] = cur; letPages[j] = pagesFor(letterMap[letters[j]].size()); cur += letPages[j]; }
				const int SEARCH_PG = cur;                                            // live search page (type to filter)
				const int PARTY_PG = cur + 1;                                         // "Party, items & shop" hub
				const int LOVE_PG = cur + 2;                                          // love-level page
				// --- live equipment search variables (all allocated in SysDatabase and unreferenced by the game; see tools/find_free_strings.py for the strings)
				const uint32_t TC[8] = {2000007, 2000016, 2000019, 2000025, 2000026, 2000049, 2000080, 2000081};   // typed characters (key codes), up to 8
				const uint32_t TN = 2000082;                                          // number of typed characters
				const uint32_t LASTK = 2100017, MC = 2100050, OFFS = 2100065, GT = 2100069, RC = 2200018;           // key held last frame, match count, result offset, give-target rank, visible rows
				const uint32_t CUR = 3000021, TMP = 3000022;                          // string variables: typed text, scratch name
				const int RES_ROWS = 11;                                              // result rows per page
				const uint32_t FB = 2200013;                                          // feedback toast: frames left to show it (allocated, unreferenced by the game)
				const int MAXR = 14;
				const uint32_t baseY = 100, stepY = 34;
				const uint32_t PIC_DIM = 99900, PIC_TITLE = 99901, PIC_ITEM = 99902, PIC_CURSOR = PIC_ITEM + MAXR, PIC_STATUS = PIC_CURSOR + 1, PIC_HELP = PIC_STATUS + 1, PIC_TOAST = PIC_HELP + 1;
				const uint32_t banMask = getenv("FM2_BAN") ? (uint32_t)std::stoul(getenv("FM2_BAN")) : 496u;
				CommonEvent& src = ce.m_events.at(119);

				// ====== live equipment search: three call-only events in blank slots ======
				// 935 REFRESH: rebuilds the result rows for the current typed text (CUR / TN) and page offset (OFFS). It scans all equipment names in an unrolled list:
				//   a name matches when TN == 0 (empty search shows everything) or when removing every occurrence of the typed text changes it (= it contains the text).
				//   Matches are counted in MC; the ones whose rank falls inside the current window are drawn directly as picture rows (picture number and y come from variables).
				// 936 GIVE: same scan, but runs the game's pickup event 197 for the match whose rank equals GT.
				// 937 KEYS: called every frame while the search page is open: reads A-Z, 0-9, space, '-', ''', Backspace, Delete, edits the typed text, rebuilds CUR, resets
				//   the offset and calls 935.
				{
					struct PK { uint32_t code; wchar_t ch; };
					static const PK printable[] = {   // Wolf key code = 100 + DirectInput scancode
						{116, L'q'}, {117, L'w'}, {118, L'e'}, {119, L'r'}, {120, L't'}, {121, L'y'}, {122, L'u'}, {123, L'i'}, {124, L'o'}, {125, L'p'},
						{130, L'a'}, {131, L's'}, {132, L'd'}, {133, L'f'}, {134, L'g'}, {135, L'h'}, {136, L'j'}, {137, L'k'}, {138, L'l'},
						{144, L'z'}, {145, L'x'}, {146, L'c'}, {147, L'v'}, {148, L'b'}, {149, L'n'}, {150, L'm'},
						{102, L'1'}, {103, L'2'}, {104, L'3'}, {105, L'4'}, {106, L'5'}, {107, L'6'}, {108, L'7'}, {109, L'8'}, {110, L'9'}, {111, L'0'},
						{157, L' '}, {112, L'-'}, {140, L'\''} };
					const int NPK = (int)(sizeof(printable) / sizeof(printable[0]));
					const uint32_t K0 = 1600000, K1 = 1600001, K2 = 1600002, K3 = 1600003;           // event-local temporaries (only 0-4 exist in these slot events)
					auto lower = [](std::wstring s) { for (auto& c : s) if (c >= L'A' && c <= L'Z') c = (wchar_t)(c + 32); return s; };
					const uint32_t PN_BASE = PIC_ITEM - 1;                                         // row k (1-based) uses picture number PIC_ITEM + k - 1

					// ---- 935 REFRESH
					{
						auto& ev = ce.m_events.at(935);
						ev.m_name = L"FM2 search refresh";
						ev.m_commands.clear();
						EB b(ev.m_commands);
						b.set(MC, 0);
						for (int r = 0; r < MAXR; r++) b.pic({32, PIC_ITEM + (uint32_t)r, 0, 1, 1, 1, 255, 100, baseY + stepY * (uint32_t)r, 85, 0}, L" ");   // blank the rows
						for (int i = 0; i < NE; i++)
						{
							const std::wstring lowName = lower(equip[i].name);
							const std::wstring rowText = std::to_wstring(equip[i].id) + L"   " + equip[i].name;
							b.set(K3, 0);                                                          // K3 = "this item matches"
							b.ifEq(TN, 0, [&] { b.set(K3, 1); }, [&]
							{
								b.setStr(TMP, lowName);
								b.setStrFromVar(TMP, CUR, 8);                                     // TMP = name without any occurrence of the typed text
								b.ifStrNe(TMP, lowName, [&] { b.set(K3, 1); });                   // changed => the name contained the text
							}, true);
							b.ifEq(K3, 1, [&]
							{
								b.add(MC, 1);
								b.set(K0, MC); b.sub(K0, OFFS);                                     // K0 = rank inside the visible window (1..)
								b.ifOp(K0, 0, 0, [&]                                              // K0 > 0 (in this engine comparison op 0 behaves as a strict ">")
								{
									b.ifOp(K0, (uint32_t)RES_ROWS, 3, [&]                          // K0 <= rows per page
									{
										b.set(K1, PN_BASE); b.add(K1, K0);                          // picture number
										b.set(K2, K0); b.sub(K2, 1); b.mul(K2, stepY); b.add(K2, baseY);   // y
										b.pic({32, K1, 0, 1, 1, 1, 255, 100, K2, 85, 0}, rowText);
									});
								});
							});
						}
						// visible rows on this page = clamp(MC - OFFS, 0, RES_ROWS)
						b.set(RC, MC); b.sub(RC, OFFS);
						b.ifOp(RC, (uint32_t)RES_ROWS, 1, [&] { b.set(RC, (uint32_t)RES_ROWS); });
						b.ifBelowZero(RC, [&] { b.set(RC, 0); });
						b.cmd(CT::Blank);
					}
					// ---- 936 GIVE (rank GT within the current filter)
					{
						auto& ev = ce.m_events.at(936);
						ev.m_name = L"FM2 search give";
						ev.m_commands.clear();
						EB b(ev.m_commands);
						b.set(K0, 0);
						for (int i = 0; i < NE; i++)
						{
							const std::wstring lowName = lower(equip[i].name);
							b.set(K3, 0);
							b.ifEq(TN, 0, [&] { b.set(K3, 1); }, [&]
							{
								b.setStr(TMP, lowName);
								b.setStrFromVar(TMP, CUR, 8);
								b.ifStrNe(TMP, lowName, [&] { b.set(K3, 1); });
							}, true);
							b.ifEq(K3, 1, [&]
							{
								b.add(K0, 1);
								b.ifEq(K0, GT, [&] { b.cmd(CT::CommonEvent, {500197, 4, 10100, 512, 384, equip[i].id}); b.pic({32, PIC_TOAST, 0, 1, 1, 1, 255, 60, baseY + stepY * MAXR + 80, 100, 0}, L"Done:  " + std::to_wstring(equip[i].id) + L"   " + equip[i].name); b.set(FB, 150); });
							});
						}
						b.cmd(CT::Blank);
					}
					// ---- 937 KEYS
					{
						auto& ev = ce.m_events.at(937);
						ev.m_name = L"FM2 search keys";
						ev.m_commands.clear();
						EB b(ev.m_commands);
						// K0 = the key pressed this frame (0 = none; if several are down the last one in the list wins), K1 = temporary read
						b.set(K0, 0);
						for (int i = 0; i < NPK; i++) { b.cmd(CT::InputKey, {K1, 4352, printable[i].code}); b.ifEq(K1, printable[i].code, [&] { b.set(K0, printable[i].code); }); }
						b.cmd(CT::InputKey, {K1, 4352, 114}); b.ifEq(K1, 114, [&] { b.set(K0, 114); });   // Backspace
						b.cmd(CT::InputKey, {K1, 4352, 311}); b.ifEq(K1, 311, [&] { b.set(K0, 311); });   // Delete (clear all)
						b.ifEq(K0, 0, [&] { b.set(LASTK, 0); }, [&]
						{
							b.ifEq(LASTK, 999, [&] { b.set(K0, 999); });                          // 999 = page just opened: treat every held key as "already held" until all are released
							b.ifEq(K0, LASTK, [] {}, [&]                                          // a NEW key press (not the same key held down)
							{
								b.set(LASTK, K0);
								// Invariant: TC[i] holds the key code of character i+1, or 0 when unused, so the text can be rebuilt without any "greater than" tests.
								b.ifEq(K0, 114, [&]
								{
									b.ifEq(TN, 0, [] {}, [&] { for (int n = 1; n <= 8; n++) b.ifEq(TN, (uint32_t)n, [&, n] { b.set(TC[n - 1], 0); }); b.sub(TN, 1); }, true);
								}, [&]
								{
									b.ifEq(K0, 311, [&] { b.set(TN, 0); for (int n = 0; n < 8; n++) b.set(TC[n], 0); }, [&]
									{
										b.ifEq(TN, 8, [] {}, [&]                                  // room for another character
										{
											b.add(TN, 1);
											for (int n = 1; n <= 8; n++) b.ifEq(TN, (uint32_t)n, [&, n] { b.set(TC[n - 1], K0); });
										}, true);
									}, true);
								}, true);
								// rebuild the typed text CUR from the stored key codes
								b.setStr(CUR, L"");
								for (int n = 1; n <= 8; n++)
									for (int k = 0; k < NPK; k++) b.ifEq(TC[n - 1], printable[k].code, [&, k] { b.setStr(CUR, std::wstring(1, printable[k].ch), 1); });
								b.set(OFFS, 0); b.set(S, 0);
								b.cmd(CT::CommonEvent, {500935, 0});
							}, true);
						}, true);
						b.cmd(CT::Blank);
					}
				}

				// --- Event A (slot 500, id < 529 so it runs before the game's own menu-key event): Insert opens the menu
				{
					auto& ev = ce.m_events.at(500);
					ev.m_unknown1 = src.m_unknown1; ev.m_unknown2 = src.m_unknown2; // parallel while V9 == 1 (field)
					ev.m_name = L"FM2 menu open";
					ev.m_commands.clear();
					EB b(ev.m_commands);
					auto openMenu = [&] { b.set(WD, 1); b.cmd(CT::Picture, {2, 99950, 0}); b.cmd(CT::Picture, {2, 99951, 0}); b.set(CL, 0); b.set(M, 1); b.set(PG, 0); b.set(S, 0); b.set(V9, 0); b.cmd(CT::BanInput, {banMask}); };
					b.edge(C0, KEY_INS, LI, openMenu);                                  // keyboard: Insert
					b.cmd(CT::InputKey, {C1, 4608, PAD_L3}); b.cmd(CT::InputKey, {C2, 4608, PAD_R3}); // controller: hold L3 + R3 (stick clicks)
					b.ifEq(C1, PAD_L3, [&] { b.ifEq(C2, PAD_R3, [&] { b.ifEq(LC, 0, [&] { b.set(LC, 1); openMenu(); }); }, [&] { b.set(LC, 0); }, true); }, [&] { b.set(LC, 0); }, true);
				}
				// --- Event F (slot 6): time freeze. Parallel while FZ == 1 (any game state): pins the clock slot and move counter.
				{
					auto& ev = ce.m_events.at(6);
					ev.m_unknown1 = (FZ << 8) | 0x22; ev.m_unknown2 = src.m_unknown2; ev.m_unknown2[1] = 1;
					ev.m_name = L"FM2 time freeze";
					ev.m_commands.clear();
					EB b(ev.m_commands);
					b.set(V22, PT); b.set(V31, PC);
					b.cmd(CT::Blank);
				}
				// --- Event Z (slot 7, id < 119 so it runs before the game's field-zoom event): keeps the chosen map zoom when the game resets V10 on area changes
				{
					auto& ev = ce.m_events.at(7);
					ev.m_unknown1 = src.m_unknown1; ev.m_unknown2 = src.m_unknown2; // parallel while V9 == 1 (field)
					ev.m_name = L"FM2 zoom pin";
					ev.m_commands.clear();
					EB b(ev.m_commands);
					b.ifEq(ZE, 1, [&] { b.set(V10, ZV); });
				}
				// --- Walking speed (set FM2_NO_SPEED=1 when building to leave this out entirely).
				// The game's walking driver is event 120: every frame it sets the walker's speed with `Move [-2]` + route "29(n)" (3 for straight moves,
				// 2 for diagonals, in its normal branch), so a speed set once from the menu was overwritten at once. Instead, each of those Move commands is
				// replaced by: if SE == 1 then (Move with the chosen speed: straight = SS, diagonal = SS-1) else (the original Move).
				if (SPEED)
				{
					auto& c120 = ce.m_events.at(120).m_commands;
					for (int idx : {20, 17, 12, 9, 6})   // highest first so the lower indices stay valid
					{
						auto mv = std::dynamic_pointer_cast<Command::CommandSpecialClasses::Move>(c120.at(idx));
						const uint32_t orig = mv->m_route.at(0).m_args.at(0);
						const bool diag = (orig == 2);
						FM2::Cmds tv; EB b(tv); b.ind = mv->m_indent;
						auto original = std::make_shared<Command::CommandSpecialClasses::Move>(*mv);
						b.ifEq(SE, 1, [&]
						{
							for (uint32_t s = 1; s <= 8; s++)
								b.ifEq(SS, s, [&, s]
								{
									auto m2 = std::make_shared<Command::CommandSpecialClasses::Move>(*mv);
									m2->m_route[0].m_args[0] = diag ? (s > 0 ? s - 1 : 0) : s;
									b.raw(m2);
								});
						}, [&] { b.raw(original); }, true);
						c120.erase(c120.begin() + idx);
						c120.insert(c120.begin() + idx, tv.begin(), tv.end());
					}
				}
				// --- Event W (slot 8): warp. AUTO-RUN event when WPF == 1. The sequence is copied from the game's own menu-driven warp (event 556, the world map):
				// black fade picture -> teleport to the staging map 28 (forces a full map reload) -> teleport to the destination -> Move -> events 122 / 584 / 582.
				{
					auto& ev = ce.m_events.at(8);
					ev.m_unknown1 = (WPF << 8) | 0x21; ev.m_unknown2 = src.m_unknown2; ev.m_unknown2[1] = 1;
					ev.m_name = L"FM2 warp";
					ev.m_commands.clear();
					EB b(ev.m_commands);
					auto mv556 = std::dynamic_pointer_cast<Command::CommandSpecialClasses::Move>(ce.m_events.at(556).m_commands.at(616)); // the Move [this event] that follows event 556's second Teleport
					b.pic({32, 99951, 0, 1, 1, 1, 255, 20, 730, 90, 0}, L"FM2 warp event running -> target #\\v9[22]");   // diagnostic
					b.set(T31, V31); b.set(T22, V22);
					b.set(WC, 0); b.set(WF, 1);                                            // arm the arrival watchdog (slot 9)
					for (int i = 0; i < NT; i++)
						b.ifEq(WP, (uint32_t)(i + 1), [&, i]
						{
							b.set(V11, 1); b.set(V9, 0);
							b.pic({0, 999999, 10, 1, 1, 1, 255, 0, 0, 200, 0}, L"Picture/まっくら.jpg");   // fade to black, as event 556 and the doors do
							b.cmd(CT::Teleport, {4294967295u, 21, 18, 28, 33});                                              // staging hop through map 28
							b.cmd(CT::Teleport, {4294967295u, targets[i].x, targets[i].y, targets[i].id, targets[i].opt | 0x20u}); // destination: keep the 'precise' bit, add the fade
							if (mv556) b.raw(std::make_shared<Command::CommandSpecialClasses::Move>(*mv556));
							b.set(2000030, 0);
							b.cmd(CT::CommonEvent, {500122, 0}); b.cmd(CT::CommonEvent, {500584, 0}); b.cmd(CT::CommonEvent, {500582, 2, 0, 0});
							b.set(2000008, 0);
							b.cmd(CT::Wait, {1});
						});
					b.set(V31, T31); b.set(V22, T22);                                      // a warp costs no time
					b.set(PT, V22); b.set(PC, V31);
					b.set(WP, 0); b.set(WPF, 0);
					b.cmd(CT::Picture, {2, 99950, 0}); b.cmd(CT::Picture, {2, 99951, 0});
					b.cmd(CT::Blank);
				}
				// --- Event D (slot 9): arrival watchdog. Parallel while WF == 1 (any state). The arrival routine (event 124) normally sets V9 = 1, erases the black
				// picture and clears V11 within a second; if V9 is still 0 after 180 frames, do those things ourselves so the player never stays frozen in the dark.
				{
					auto& ev = ce.m_events.at(9);
					ev.m_unknown1 = (WF << 8) | 0x22; ev.m_unknown2 = src.m_unknown2; ev.m_unknown2[1] = 1;
					ev.m_name = L"FM2 warp watchdog";
					ev.m_commands.clear();
					EB b(ev.m_commands);
					b.add(WC, 1);
					b.ifEq(WC, 180, [&]
					{
						b.ifEq(V9, 0, [&] { b.set(V9, 1); b.set(V11, 0); b.set(2000013, 1); b.cmd(CT::Picture, {2, 999999, 15, 35}); });
						b.set(WF, 0); b.set(WC, 0);
					});
				}
				if (getenv("FM2_WARPDBG")) // debug readout of the arrival flags (slot 10; runs in any state once the menu has been opened, WD = 1)
				{
					auto& ev = ce.m_events.at(10);
					ev.m_unknown1 = (WD << 8) | 0x22; ev.m_unknown2 = src.m_unknown2; ev.m_unknown2[1] = 1;
					ev.m_name = L"FM2 warp debug";
					ev.m_commands.clear();
					EB b(ev.m_commands);
					b.pic({32, 99972, 0, 1, 1, 1, 255, 6, 2, 70, 0}, L"V9=\\v[9] V11=\\v[11] V13=\\v[13] V17=\\v[17] V18=\\v[18] WF=\\v6[1] WC=\\v6[2]");
					b.cmd(CT::Blank);
				}
				if (getenv("FM2_PADPROBE")) // controller probe overlay (slot 11): shows the pad button code and a bitmask of which specific pad codes 300..315 read as pressed
				{
					auto& ev = ce.m_events.at(11);
					ev.m_unknown1 = src.m_unknown1; ev.m_unknown2 = src.m_unknown2;
					ev.m_name = L"FM2 pad probe";
					ev.m_commands.clear();
					EB b(ev.m_commands);
					b.cmd(CT::InputKey, {C1, 512, 100});                               // pad, non-specific: code of the pressed pad button
					b.set(C2, 0);
					for (uint32_t i = 0; i < 16; i++)
					{
						b.cmd(CT::InputKey, {C3, 4608, 300 + i});                      // pad, specific button 300+i
						b.ifEq(C3, 300 + i, [&] { b.add(C2, 1u << i); });
					}
					b.pic({32, 99971, 0, 1, 1, 1, 255, 20, 60, 110, 0}, L"PAD code=\\cself[1]   mask(300..315)=\\cself[2]");
					b.cmd(CT::Blank);
				}
				if (getenv("FM2_TIMEDBG")) // debug overlay of the time system variables (slot 82, field-parallel)
				{
					auto& ev = ce.m_events.at(82);
					ev.m_unknown1 = src.m_unknown1; ev.m_unknown2 = src.m_unknown2;
					ev.m_name = L"FM2 time debug";
					ev.m_commands.clear();
					EB b(ev.m_commands);
					b.pic({32, 99970, 0, 1, 1, 1, 255, 230, 2, 70, 0}, L"T=\\v[22] C=\\v[31] SC=\\v[23] SH=\\v[32] NX=\\v[14] TS=\\v[0] FZ=\\v9[10] Z=\\v[10]");
					b.cmd(CT::Blank);
				}
				if (getenv("FM2_TESTTICK")) // test aid: pushes the clock +1 every frame (slot 929 runs after the freeze event in slot 6)
				{
					auto& ev = ce.m_events.at(929);
					ev.m_unknown1 = src.m_unknown1; ev.m_unknown2 = src.m_unknown2;
					ev.m_name = L"FM2 test tick";
					ev.m_commands.clear();
					EB b(ev.m_commands);
					const uint32_t cap = getenv("FM2_TICKCAP") ? (uint32_t)std::stoul(getenv("FM2_TICKCAP")) : 11u;
					b.add(V22, 1); b.ifEq(V22, cap + 1, [&] { b.set(V22, cap); });
				}
				// --- Event B (slot 934): the menu itself, parallel while M == 1
				{
					auto& ev = ce.m_events.at(934);
					ev.m_unknown1 = (M << 8) | 0x22; ev.m_unknown2 = src.m_unknown2; ev.m_unknown2[1] = 1;
					ev.m_name = L"FM2 menu";
					ev.m_commands.clear();
					EB b(ev.m_commands);

					// closing hides the UI at once but keeps the freeze until X/Insert/Z are released, so the
					// closing keypress can't leak into the game's own menu
					auto close = [&]
					{
						b.set(CL, 1);
						for (uint32_t n = PIC_DIM; n <= PIC_TOAST; n++) b.cmd(CT::Picture, {2, n, 0});
						b.set(FB, 0);
					};
					auto syncPins = [&] { b.set(PT, V22); b.set(PC, V31); };
					auto refreshClock = [&] { b.cmd(CT::CommonEvent, {500123, 0}); b.cmd(CT::CommonEvent, {500584, 0}); }; // lighting + HUD redraw (as event 122 does)
					auto setSlot = [&](uint32_t slot) { b.set(V22, slot); syncPins(); refreshClock(); };
					auto goPage = [&](uint32_t pg) { b.set(PS, S); b.set(PG, pg); b.set(S, 0); };   // from the main page: remember the cursor
					auto setPage = [&](uint32_t pg) { b.set(PG, pg); b.set(S, 0); };                // between sub pages
					auto goMain = [&] { b.set(PG, 0); b.set(S, PS); };
					auto goCurrency = [&] { b.set(PG, 1); b.set(S, CUR_CUSTOM_ROW); };               // back from the custom-amount page (opened from Currency)

					struct Row { std::wstring text; std::function<void()> act; bool nofb = false; };   // nofb: no feedback toast (navigation rows, rows that close the menu)
					struct Pg { std::wstring title; std::vector<Row> rows; };
					std::vector<Pg> pages;

					// page 0: main
					pages.push_back({L"FM2 CHEAT MENU", {
						{L"Currency & items  >", [&] { goPage(1); }},
						{L"Time  >", [&] { goPage(2); }},
						{L"Unlocks  >", [&] { goPage(3); }},
						{L"Dungeon floor-skip flags (untested)  >", [&] { goPage(4); }},
						{L"Map zoom  >", [&] { goPage(5); }},
												{L"Warp to map  >", [&] { goPage(WARP0); }},
						{L"Close menu", close, true},
					}});
					if (SPEED) pages[0].rows.insert(pages[0].rows.begin() + 5, Row{L"Move speed  >", [&] { goPage(6); }});   // after Map zoom
					pages[0].rows.insert(pages[0].rows.begin() + (SPEED ? 6 : 5), Row{L"Party, items & shop  >", [&] { goPage(PARTY_PG); }});
					// page 1: currency & items
					pages.push_back({L"CURRENCY & ITEMS", {
						{L"+10000 AP", [&] { b.add(AP, 10000); }},
						{L"+100000 AP", [&] { b.add(AP, 100000); }},
						{L"+10000 Linel", [&] { b.add(LINEL, 10000); }},
						{L"+100000 Linel", [&] { b.add(LINEL, 100000); }},
						{L"+1000 casino coins (untested)",[&] { b.add(2000047, 1000); }},
						{L"+10000 casino coins (untested)",[&] { b.add(2000047, 10000); }},
						{L"+5 meal tickets (untested)",[&] { b.add(2000045, 5); }},
						{L"+10 lottery tickets", [&] { b.add(2300012, 10); }},
						{L"Custom amount (add or subtract)  >", [&] { b.set(PG, (uint32_t)CUSTOM_PG); b.set(S, 0); b.ifEq(STEP, 0, [&] { b.set(STEP, 100); }); }},   // keeps PS (the main-menu cursor) untouched
						{L"<  Back", goMain},
					}});
					// page 2: time
					pages.push_back({L"TIME", {
						{L"Morning (slot 0)", [&] { setSlot(0); }},
						{L"Midday (slot 5)", [&] { setSlot(5); }},
						{L"Night (slot 10)", [&] { setSlot(10); }},
						{L"+1 slot", [&] { b.add(V22, 1); b.ifEq(V22, 14, [&] { b.set(V22, 13); }); syncPins(); refreshClock(); }},
						{L"-1 slot", [&] { b.ifEq(V22, 0, [] {}, [&] { b.sub(V22, 1); }, true); syncPins(); refreshClock(); }},
						{L"Freeze time on/off", [&] { b.ifEq(FZ, 0, [&] { b.set(FZ, 1); syncPins(); }, [&] { b.set(FZ, 0); }, true); }},
						{L"Reset move counter", [&] { b.set(V31, 0); syncPins(); refreshClock(); }},
						{L"New day (morning + reset daily limits)", [&] { b.set(V22, 0); b.set(V31, 0); b.set(2000032, 0); b.set(2000033, 0); b.set(2000034, 0); syncPins(); refreshClock(); }},
						{L"<  Back", goMain},
					}});
					// page 3: unlock flags (the game writes 1 to each of these when the feature is unlocked)
					pages.push_back({L"UNLOCKS", {
						{L"World map access", [&] { b.set(2000021, 1); }},
						{L"Wechina tour", [&] { b.set(2000046, 1); }},
						{L"Linel circle", [&] { b.set(2200085, 1); }},
						{L"Assassin display", [&] { b.set(2200086, 1); }},
						{L"Subjugation challenge", [&] { b.set(2200090, 1); }},
						{L"Jasmine scene unlock", [&] { b.set(2900031, 1); }},
						{L"Wechina scene unlock", [&] { b.set(2900035, 1); }},
						// the scene album's "seen" flags are CDB 31 item n field 0, kept in the COMMON save slot 101 (the album reads them with command 221 slot 101; each scene's
						// event writes its flag with command 222 [1, 101, 1131000000 + n*100, 0]). Album screens 825/842 show scenes 1..45.
						{L"Unlock all album scenes (1-45)", [&] { for (uint32_t n = 1; n <= 45; n++) b.cmd(CT::SaveGame, {1, 101, 1131000000u + n * 100u, 0}); }},
						{L"<  Back", goMain},
					}});
					// page 4: dungeon floor-skip flags (names: 50F/70F/100F skip; the game tests each == 1)
					pages.push_back({L"FLOOR-SKIP FLAGS (untested)", {
						{L"50F skip flag  ON", [&] { b.set(2000077, 1); }},
						{L"70F skip flag  ON", [&] { b.set(2000079, 1); }},
						{L"100F skip flag  ON", [&] { b.set(2000071, 1); }},
						{L"All three skip flags  OFF", [&] { b.set(2000077, 0); b.set(2000079, 0); b.set(2000071, 0); }},
						{L"<  Back", goMain},
					}});
					// page 5: map zoom (V10 is the field zoom percentage the game itself sets, default 200)
					pages.push_back({L"MAP ZOOM", {
						{L"100%", [&] { b.set(ZV, 100); b.set(ZE, 1); b.set(V10, 100); }},
						{L"150%", [&] { b.set(ZV, 150); b.set(ZE, 1); b.set(V10, 150); }},
						{L"200%  (game default; stop keeping my zoom)", [&] { b.set(ZE, 0); b.set(V10, 200); }},
						{L"250%", [&] { b.set(ZV, 250); b.set(ZE, 1); b.set(V10, 250); }},
						{L"300%", [&] { b.set(ZV, 300); b.set(ZE, 1); b.set(V10, 300); }},
						{L"<  Back", goMain},
					}});
					// page 6: walking speed (patched into the game's walking event 120; 3 is the game's normal speed, diagonals use speed-1)
					if (SPEED)
					{
						auto spd = [&](uint32_t n) { return std::function<void()>([&, n] { b.set(SS, n); b.set(SE, 1); }); };
						pages.push_back({L"MOVE SPEED (3 = the game's normal speed)", {
							{L"Speed 2 (slow)", spd(2)},
							{L"Speed 3 (normal)", spd(3)},
							{L"Speed 4", spd(4)},
							{L"Speed 5", spd(5)},
							{L"Speed 6 (the dungeon's speed)", spd(6)},
							{L"Speed 7 (may be capped by the engine)", spd(7)},
							{L"Speed 8 (may be capped by the engine)", spd(8)},
							{L"Back to the game's own speed", [&] { b.set(SE, 0); }},
							{L"<  Back", goMain},
						}});
					}
					// custom amount: edit a signed number, then add it to a currency. Negative amounts subtract; results are clamped at 0 because it is unknown how the game
					// treats negative AP / Linel. (Constants >= 1,000,000 would be read as variable ids, so the step sizes stop at 100000.)
					{
						auto applyTo = [&](uint32_t var) { return std::function<void()>([&, var] { b.add(var, CA); b.ifBelowZero(var, [&, var] { b.set(var, 0); }); }); };
						pages.push_back({L"CUSTOM AMOUNT", {
							{L"Amount: \\v5[0]    (Left / Right to change)", [] {}},
							{L"Step per Left/Right press: \\v7[0]    (Confirm: next step)", [&]
								{
									b.set(C3, STEP);                                              // snapshot, so the chain below can't cascade
									b.ifEq(C3, 1, [&] { b.set(STEP, 10); });
									b.ifEq(C3, 10, [&] { b.set(STEP, 100); });
									b.ifEq(C3, 100, [&] { b.set(STEP, 1000); });
									b.ifEq(C3, 1000, [&] { b.set(STEP, 10000); });
									b.ifEq(C3, 10000, [&] { b.set(STEP, 100000); });
									b.ifEq(C3, 100000, [&] { b.set(STEP, 1); });
									b.ifEq(C3, 0, [&] { b.set(STEP, 100); });
								}},
							{L"Flip sign (+ / -)", [&] { b.set(C3, CA); b.set(CA, 0); b.sub(CA, C3); }},
							{L"Set amount to 0", [&] { b.set(CA, 0); }},
							{L"Add amount to AP", applyTo(AP)},
							{L"Add amount to Linel", applyTo(LINEL)},
							{L"Add amount to lottery tickets", applyTo(2300012)},
							{L"Add amount to casino coins (untested)", applyTo(2000047)},
							{L"Add amount to meal tickets (untested)", applyTo(2000045)},
							{L"<  Back", goCurrency},
						}});
					}
					// pages 7..: warp targets (entry coordinates copied from the game's own Teleport commands)
					for (int p = 0; p < WARP_PAGES; p++)
					{
						Pg pg;
						pg.title = L"WARP  " + std::to_wstring(p + 1) + L"/" + std::to_wstring(WARP_PAGES);
						for (int i = p * PER; i < NT && i < (p + 1) * PER; i++)
						{
							const WT& t = targets[i];
							pg.rows.push_back({std::wstring(t.name), [&, tag = (uint32_t)(i + 1)] { b.set(WP, tag); close(); }, true});   // WP = target index + 1, so that map id 0 ("Rouge's Room") is not mistaken for "no warp pending"
						}
						if (p + 1 < WARP_PAGES) pg.rows.push_back({L"Next page  >>", [&, p] { setPage(WARP0 + p + 1); }});
						if (p > 0) pg.rows.push_back({L"<<  Previous page", [&, p] { setPage(WARP0 + p - 1); }});
						pg.rows.push_back({L"<  Back to main", goMain});
						pages.push_back(pg);
					}

					auto goTo = [&](int pg, int row) { return std::function<void()>([&, pg, row] { b.set(PG, (uint32_t)pg); b.set(S, (uint32_t)row); }); };   // jump to a page with the cursor on a row
					auto goPartyRow = [&](uint32_t row) { return goTo(PARTY_PG, (int)row); };
					// opening the live search: clear the typed text, show every item, and wait until all keys are released (LASTK = 999) so the key that selected the row isn't typed
					std::function<void()> enterSearch = [&]
					{
						b.set(PG, (uint32_t)SEARCH_PG); b.set(S, 0); b.set(TN, 0); for (int n = 0; n < 8; n++) b.set(TC[n], 0); b.setStr(CUR, L""); b.set(OFFS, 0); b.set(LASTK, 999);
						b.cmd(CT::CommonEvent, {500935, 0});
					};
					std::map<int, std::pair<int, int>> parentOf;      // page -> (parent page, cursor row there); pages not listed return to the main menu
					std::vector<std::pair<int, int>> groups;           // runs of pages that Left/Right flip through (wrapping inside the run)
					parentOf[CUSTOM_PG] = {1, 8};
					if (WARP_PAGES > 1) groups.push_back({WARP0, WARP_PAGES});
					// Equipment rows call the game's own "obtain equipment" event 197 ([500197, 4, 10100, 512, 384, equipment id], the same call the battle victory and
					// dungeon events make), so the inventory and the pickup effect are handled by the game.
					auto equipRow = [&](int idx) { return Row{std::to_wstring(equip[idx].id) + L"   " + equip[idx].name, std::function<void()>([&, id = equip[idx].id] { b.cmd(CT::CommonEvent, {500197, 4, 10100, 512, 384, id}); })}; };
					// 1) everything, in the game's order
					groups.push_back({EQ0, EQ_PAGES});
					for (int p = 0; p < EQ_PAGES; p++)
					{
						Pg pg;
						pg.title = L"GIVE EQUIPMENT: EVERYTHING  " + std::to_wstring(p + 1) + L"/" + std::to_wstring(EQ_PAGES) + L"    (Left / Right: page)";
						for (int i = p * PER; i < NE && i < (p + 1) * PER; i++) pg.rows.push_back(equipRow(i));
						pg.rows.push_back({L"<  Back", goTo(EQHUB, 0)});
						parentOf[EQ0 + p] = {EQHUB, 0};
						pages.push_back(pg);
					}
					// 2) the hub (EQHUB)
					pages.push_back({L"GIVE EQUIPMENT", {
						{L"Browse everything (by page)  >", goTo(EQ0, 0)},
						{L"Filter by type (swords, armor, ...)  >", goTo(TYPELIST, 0)},
						{L"Find by first letter (A-Z)  >", goTo(LS0, 0)},
						{L"Search by typing (live, keyboard)  >", enterSearch},
						{L"<  Back", goPartyRow(0)},
					}});
					parentOf[EQHUB] = {PARTY_PG, 0};
					parentOf[SEARCH_PG] = {EQHUB, 3};
					// 3) the list of types (TYPELIST) and one run of pages per type
					{
						Pg pg; pg.title = L"FILTER BY TYPE";
						for (size_t c = 0; c < cats.size(); c++)
							pg.rows.push_back({std::wstring(cats[c].name) + L"   (" + std::to_wstring(catItems[c].size()) + L")", goTo(catFirst[c], 0)});
						pg.rows.push_back({L"<  Back", goTo(EQHUB, 1)});
						pages.push_back(pg);
						parentOf[TYPELIST] = {EQHUB, 1};
					}
					for (size_t c = 0; c < cats.size(); c++)
					{
						if (catPages[c] > 1) groups.push_back({catFirst[c], catPages[c]});
						for (int p = 0; p < catPages[c]; p++)
						{
							Pg pg;
							pg.title = std::wstring(cats[c].name) + L"   " + std::to_wstring(p + 1) + L"/" + std::to_wstring(catPages[c]) + (catPages[c] > 1 ? L"    (Left / Right: page)" : L"");
							for (size_t i = p * PER; i < catItems[c].size() && i < (size_t)(p + 1) * PER; i++) pg.rows.push_back(equipRow(catItems[c][i]));
							pg.rows.push_back({L"<  Back", goTo(TYPELIST, (int)c)});
							parentOf[catFirst[c] + p] = {TYPELIST, (int)c};
							pages.push_back(pg);
						}
					}
					// 4) find by first letter: letter-select pages (13 letters each), then one run of pages per letter
					if (LSN > 1) groups.push_back({LS0, LSN});
					for (int s = 0; s < LSN; s++)
					{
						Pg pg;
						pg.title = L"FIND BY FIRST LETTER   " + std::to_wstring(s + 1) + L"/" + std::to_wstring(LSN) + (LSN > 1 ? L"    (Left / Right: more letters)" : L"");
						for (int j = s * LPS; j < (int)letters.size() && j < (s + 1) * LPS; j++)
							pg.rows.push_back({std::wstring(1, letters[j]) + L"   (" + std::to_wstring(letterMap[letters[j]].size()) + L" items)", goTo(letFirst[j], 0)});
						pg.rows.push_back({L"<  Back", goTo(EQHUB, 2)});
						parentOf[LS0 + s] = {EQHUB, 2};
						pages.push_back(pg);
					}
					for (size_t j = 0; j < letters.size(); j++)
					{
						auto& items = letterMap[letters[j]];
						if (letPages[j] > 1) groups.push_back({letFirst[j], letPages[j]});
						for (int p = 0; p < letPages[j]; p++)
						{
							Pg pg;
							pg.title = std::wstring(L"ITEMS STARTING WITH ") + letters[j] + L"   " + std::to_wstring(p + 1) + L"/" + std::to_wstring(letPages[j]) + (letPages[j] > 1 ? L"    (Left / Right: page)" : L"");
							for (size_t i = p * PER; i < items.size() && i < (size_t)(p + 1) * PER; i++) pg.rows.push_back(equipRow(items[i]));
							pg.rows.push_back({L"<  Back", goTo(LS0 + (int)j / LPS, (int)j % LPS)});
							parentOf[letFirst[j] + p] = {LS0 + (int)j / LPS, (int)j % LPS};
							pages.push_back(pg);
						}
					}
					// 4b) live search page (SEARCH_PG): no static rows; the result rows are drawn by event 935 and handled by special code below
					pages.push_back({L"FIND EQUIPMENT", {}});
					if ((int)pages.size() - 1 != SEARCH_PG) throw std::runtime_error("search page index mismatch");
					// 5) Party hub (PARTY_PG)
					pages.push_back({L"PARTY, ITEMS & SHOP", {
						{L"Give equipment  >", goTo(EQHUB, 0)},
						{L"Love level  >", goTo(LOVE_PG, 0)},
						// runtime table 5 "party member skills learned / power": one field "learned?" per entry, 80 entries
						{L"Learn all skills", [&] { for (uint32_t i = 0; i < 80; i++) b.dbWrite(5, i, 0, 1); }},
						// runtime table 6 "Sebas shop owned count": fields owned? / purchasable? / show description?; field 1 = purchasable for every named perk
						{L"Shop: make every perk purchasable", [&] { for (int i = 0; i < NPERK; i++) b.dbWrite(6, perkIds[i], 1, 1); }},
						{L"<  Back", goMain},
					}});
					// 6) Love level (LOVE_PG): runtime table 9 "party character stats", item = character, field 12 = love level, 13 = love exp, 23 = level lock released.
					{
						struct HR { uint32_t item; const wchar_t* name; };
						static const HR heroines[] = {{1, L"Rouge"}, {2, L"Romario"}, {3, L"Sylphina"}, {4, L"Lillie"}, {5, L"Memel"}, {6, L"Nagi"}};
						auto setLove = [&](uint32_t item) { b.dbWrite(9, item, 12, TL); b.dbWrite(9, item, 13, 0); b.dbWrite(9, item, 23, 1); };
						Pg pg; pg.title = L"LOVE LEVEL";
						pg.rows.push_back({L"Target level: \\v1[5]    (Left / Right: 0 to 10)", [] {}});
						for (auto& h : heroines)
							pg.rows.push_back({std::wstring(h.name) + L"   ->  target level", [&, item = h.item] { setLove(item); }});
						pg.rows.push_back({L"All of the above  ->  target level", [&] { for (auto& h : heroines) setLove(h.item); }});
						pg.rows.push_back({L"<  Back", goPartyRow(1)});
						parentOf[LOVE_PG] = {PARTY_PG, 1};
						pages.push_back(pg);
					}

					auto draw = [&]
					{
						b.pic({0, PIC_DIM, 0, 1, 1, 1, 150, 0, 0, 200, 0}, L"Picture/まっくら.jpg");
						for (size_t p = 0; p < pages.size(); p++)
							b.ifEq(PG, (uint32_t)p, [&, p]
							{
								if ((int)p == SEARCH_PG)    // result rows are drawn by event 935; only the title shows the typed text and the match count
									b.pic({32, PIC_TITLE, 0, 1, 1, 1, 255, 60, 30, 120, 0}, L"FIND EQUIPMENT   search: \\s[21]_    (\\v1[50] found)");
								else
								{
									b.pic({32, PIC_TITLE, 0, 1, 1, 1, 255, 60, 30, 120, 0}, pages[p].title);
									for (int i = 0; i < MAXR; i++)
										b.pic({32, PIC_ITEM + (uint32_t)i, 0, 1, 1, 1, 255, 100, baseY + stepY * (uint32_t)i, 85, 0}, i < (int)pages[p].rows.size() ? pages[p].rows[i].text : std::wstring(L" "));
								}
							});
						b.set(C4, S); b.mul(C4, stepY); b.add(C4, baseY);      // cursor y = baseY + stepY*S
						b.pic({32, PIC_CURSOR, 0, 1, 1, 1, 255, 60, C4, 85, 0}, L">");
						b.pic({32, PIC_STATUS, 0, 1, 1, 1, 255, 60, baseY + stepY * MAXR + 14, 75, 0}, L"AP \\v[1]  Linel \\v[2]  Time slot \\v[22]/13  Freeze \\v9[10]  Zoom \\v[10]");
						b.pic({32, PIC_HELP, 0, 1, 1, 1, 255, 60, baseY + stepY * MAXR + 44, 70, 0}, L"Up/Down: select   Left/Right: page / amount   Confirm (Z): use   Cancel (X): back/close");
						b.ifEq(PG, (uint32_t)SEARCH_PG, [&] { b.pic({32, PIC_HELP, 0, 1, 1, 1, 255, 60, baseY + stepY * MAXR + 44, 70, 0}, L"Type to filter (letters, digits, space)   Backspace: erase   Delete: clear   Up/Down: select   Left/Right: page   Enter: give   Esc: back"); });
					};

					// feedback toast: a line under the help text that stays ~2.5 seconds (FB counts the frames down in the active loop; close() erases it)
					auto toast = [&](const std::wstring& msg)
					{
						b.pic({32, PIC_TOAST, 0, 1, 1, 1, 255, 60, baseY + stepY * MAXR + 80, 100, 0}, msg);
						b.set(FB, 150);
					};
					// the message for a row after its action ran (empty = none): "Done: <row text>" plus the new value where there is one; the text is evaluated after the action
					auto toastFor = [&](const Row& r) -> std::wstring
					{
						const std::wstring& t = r.text;
						auto starts = [&](const wchar_t* x) { return t.rfind(x, 0) == 0; };
						auto has = [&](const wchar_t* x) { return t.find(x) != std::wstring::npos; };
						if (r.nofb || t.empty() || t[0] == L'<' || t.back() == L'>' || starts(L"Next page") || starts(L"Amount:") || starts(L"Target level:")) return L"";
						if (starts(L"Step per")) return L"Step is now \\v7[0]";
						if (starts(L"Flip sign") || starts(L"Set amount")) return L"Amount is now \\v5[0]";
						if (has(L"Freeze time")) return L"Time freeze is now \\v9[10]   (1 = on, 0 = off)";
						if (starts(L"+1 slot") || starts(L"-1 slot") || has(L"(slot ")) return L"Done: " + t + L"   (time slot now \\v[22])";
						std::wstring m = L"Done:  " + t;
						if (has(L" AP")) m += L"   (AP now \\v[1])";
						else if (has(L"Linel")) m += L"   (Linel now \\v[2])";
						else if (has(L"casino coins")) m += L"   (now \\v[47])";
						else if (has(L"meal tickets")) m += L"   (now \\v[45])";
						else if (has(L"lottery tickets")) m += L"   (now \\v3[12])";
						return m;
					};

					// Z dispatch: page -> row
					auto onConfirm = [&]
					{
						// Dispatch on a SNAPSHOT of page and row. Without it, an action that changes the page (e.g. "Warp to map >")
						// is followed in the same frame by the checks of the later pages, which then see the new page and run its first row
						// (that closed the menu on entering the warp page, and silently applied the first zoom/speed/unlock/etc. row on entering those pages).
						b.set(C1, PG); b.set(C2, S);
						for (size_t p = 0; p < pages.size(); p++)
							b.ifEq(C1, (uint32_t)p, [&, p]
							{
								for (size_t i = 0; i < pages[p].rows.size(); i++)
									b.ifEq(C2, (uint32_t)i, [&, p, i]
									{
										pages[p].rows[i].act();
										std::wstring msg = toastFor(pages[p].rows[i]);
										if (!msg.empty()) toast(msg);
									});
							});
						// live search page: give the item whose rank among the matches is OFFS + S + 1 (event 936 scans the same filter)
						b.ifEq(C1, (uint32_t)SEARCH_PG, [&]
						{
							b.ifEq(RC, 0, [] {}, [&] { b.set(GT, OFFS); b.add(GT, C2); b.add(GT, 1); b.cmd(CT::CommonEvent, {500936, 0}); }, true);
						});
					};
					auto onUp = [&]
					{
						for (size_t p = 0; p < pages.size(); p++)
						{
							if ((int)p == SEARCH_PG) continue;
							b.ifEq(PG, (uint32_t)p, [&, p] { b.ifEq(S, 0, [&, p] { b.set(S, (uint32_t)pages[p].rows.size() - 1); }, [&] { b.sub(S, 1); }, true); });
						}
						b.ifEq(PG, (uint32_t)SEARCH_PG, [&]
						{
							b.ifEq(S, 0, [&] { b.set(S, RC); b.ifEq(RC, 0, [] {}, [&] { b.sub(S, 1); }, true); }, [&] { b.sub(S, 1); }, true);
						});
					};
					auto onDown = [&]
					{
						for (size_t p = 0; p < pages.size(); p++)
						{
							if ((int)p == SEARCH_PG) continue;
							b.ifEq(PG, (uint32_t)p, [&, p] { b.add(S, 1); b.ifEq(S, (uint32_t)pages[p].rows.size(), [&] { b.set(S, 0); }); });
						}
						b.ifEq(PG, (uint32_t)SEARCH_PG, [&] { b.add(S, 1); b.ifEq(S, RC, [&] { b.set(S, 0); }); b.ifEq(RC, 0, [&] { b.set(S, 0); }); });
					};
					// search results: Left / Right move one page of matches (OFFS = number of matches skipped) and redraw
					auto searchPage = [&](bool next)
					{
						if (next) { b.set(C3, MC); b.sub(C3, OFFS); b.ifOp(C3, (uint32_t)RES_ROWS, 0, [&] { b.add(OFFS, (uint32_t)RES_ROWS); b.set(S, 0); b.cmd(CT::CommonEvent, {500935, 0}); }); }
						else b.ifOp(OFFS, 0, 0, [&] { b.sub(OFFS, (uint32_t)RES_ROWS); b.set(S, 0); b.cmd(CT::CommonEvent, {500935, 0}); });
					};
					// Left / Right: flip through a run of pages (warp, equipment ...; wraps around inside the run); on the custom-amount page's first row they change
					// the amount by the chosen step; on the love page's first row they change the target level. Dispatch on snapshots (C1 = page, C2 = row).
					auto onLeft = [&]
					{
						b.set(C1, PG); b.set(C2, S);
						for (auto& g : groups)
							for (int p = 0; p < g.second; p++)
								b.ifEq(C1, (uint32_t)(g.first + p), [&, g, p] { b.set(PG, (uint32_t)(g.first + (p + g.second - 1) % g.second)); b.set(S, 0); });
						b.ifEq(C1, (uint32_t)SEARCH_PG, [&] { searchPage(false); });
						b.ifEq(C1, (uint32_t)CUSTOM_PG, [&] { b.ifEq(C2, 0, [&] { b.sub(CA, STEP); }); });
						b.ifEq(C1, (uint32_t)LOVE_PG, [&] { b.ifEq(C2, 0, [&] { b.ifEq(TL, 0, [] {}, [&] { b.sub(TL, 1); }, true); }); });   // love target level 0..10
					};
					auto onRight = [&]
					{
						b.set(C1, PG); b.set(C2, S);
						for (auto& g : groups)
							for (int p = 0; p < g.second; p++)
								b.ifEq(C1, (uint32_t)(g.first + p), [&, g, p] { b.set(PG, (uint32_t)(g.first + (p + 1) % g.second)); b.set(S, 0); });
						b.ifEq(C1, (uint32_t)SEARCH_PG, [&] { searchPage(true); });
						b.ifEq(C1, (uint32_t)CUSTOM_PG, [&] { b.ifEq(C2, 0, [&] { b.add(CA, STEP); }); });
						b.ifEq(C1, (uint32_t)LOVE_PG, [&] { b.ifEq(C2, 0, [&] { b.add(TL, 1); b.ifEq(TL, 11, [&] { b.set(TL, 10); }); }); });
					};
					// cancel = "go to the parent page": the main page closes; pages listed in parentOf go to their parent (cursor on the row they were opened from);
					// everything else goes to the main page. Dispatch on a snapshot of the page so the page change can't cascade.
					auto onCancel = [&]
					{
						b.set(C1, PG);
						for (int p = 0; p < (int)pages.size(); p++)
						{
							if (p == 0) b.ifEq(C1, 0, close);
							else if (parentOf.count(p)) b.ifEq(C1, (uint32_t)p, goTo(parentOf[p].first, parentOf[p].second));
							else b.ifEq(C1, (uint32_t)p, goMain);
						}
					};
					auto finalize = [&]
					{
						b.set(M, 0); b.set(CL, 0); b.set(V9, 1); b.cmd(CT::BanInput, {247});
						b.set(LI, 0); b.set(LX, 0); b.set(LZ, 0); b.set(LU, 0); b.set(LD, 0); b.set(LC, 0); b.set(LL, 0); b.set(LR, 0); b.set(HC, 0);
						// pending warp: hand over to the auto-run warp event (slot 8), which does the Teleport
						b.ifEq(WP, 0, [] {}, [&]
						{
							b.set(WPF, 1);   // (the on-screen "warp queued" diagnostic was removed once warping was confirmed working)
						}, true);
					};
					auto active = [&]
					{
						draw();
						// feedback toast timer: erase the message when its frames run out
						b.ifOp(FB, 0, 0, [&] { b.sub(FB, 1); b.ifEq(FB, 0, [&] { b.cmd(CT::Picture, {2, PIC_TOAST, 0}); }); });
						// live search: the key reader runs every frame while the search page is open (it refreshes the results itself); elsewhere LASTK stays 0
						b.ifEq(PG, (uint32_t)SEARCH_PG, [&] { b.cmd(CT::CommonEvent, {500937, 0}); }, [&] { b.set(LASTK, 0); }, true);
						// BASIC input = the engine's unified keyboard + controller input (the game's own pad config feeds it):
						// result 8 = up, 2 = down, 10 = confirm, 11 = cancel (0 = nothing); works while BanInput is active (verified)
						b.cmd(CT::InputKey, {C0, BASIC_DIR4_OK_CANCEL});
						b.cmd(CT::InputKey, {C4, 4352, KEY_INS});
						b.edgeVal(C0, 8, LU, onUp);
						b.edgeVal(C0, 2, LD, onDown);
						b.edgeVal(C0, 4, LL, onLeft);                                    // basic input 4 = left, 6 = right
						b.edgeVal(C0, 6, LR, onRight);
						// Holding Left/Right on the amount row: the first press acts at once (above); after ~15 frames it repeats every 3rd frame,
						// and after ~60 frames every frame. HC counts the frames held. Only the custom-amount page's first row reacts.
						auto repeatFire = [&](std::function<void()> amountAct)
						{
							b.set(C1, PG); b.set(C2, S);
							b.ifEq(C1, (uint32_t)CUSTOM_PG, [&, amountAct]
							{
								b.ifEq(C2, 0, [&, amountAct]
								{
									b.ifGt(HC, 60, amountAct, [&, amountAct]
									{
										b.ifGt(HC, 15, [&, amountAct] { b.set(C3, HC); b.mod(C3, 3); b.ifEq(C3, 0, amountAct); }, [] {}, false);
									}, true);
								});
							});
						};
						b.ifEq(C0, 4, [&] { b.add(HC, 1); repeatFire([&] { b.sub(CA, STEP); }); },
							[&] { b.ifEq(C0, 6, [&] { b.add(HC, 1); repeatFire([&] { b.add(CA, STEP); }); }, [&] { b.set(HC, 0); }, true); }, true);
						// Z / X / Space are also letters to type on the search page: while a typing key is down (LASTK != 0) the basic confirm / cancel are ignored there
						b.edgeVal(C0, 10, LZ, [&] { b.ifEq(LASTK, 0, onConfirm); });
						b.edgeVal(C0, 11, LX, [&] { b.ifEq(LASTK, 0, onCancel); });
						b.ifEq(C4, KEY_INS, [&] { b.ifEq(LI, 0, [&] { b.set(LI, 1); close(); }); }, [&] { b.set(LI, 0); }, true);
						// controller: L3 + R3 again closes the menu
						b.cmd(CT::InputKey, {C1, 4608, PAD_L3}); b.cmd(CT::InputKey, {C2, 4608, PAD_R3});
						b.ifEq(C1, PAD_L3, [&] { b.ifEq(C2, PAD_R3, [&] { b.ifEq(LC, 0, [&] { b.set(LC, 1); close(); }); }, [&] { b.set(LC, 0); }, true); }, [&] { b.set(LC, 0); }, true);
					};
					auto closing = [&]
					{
						b.cmd(CT::InputKey, {C0, BASIC_DIR4_OK_CANCEL}); b.cmd(CT::InputKey, {C1, 4352, KEY_INS}); b.cmd(CT::InputKey, {C2, 4608, PAD_L3});
						b.ifEq(C0, 0, [&] { b.ifEq(C1, 0, [&] { b.ifEq(C2, 0, finalize); }); });
					};
					b.ifEq(CL, 0, active, closing, true);
				}
			}
			else if (variant == "P0") // first cheat: F9 (key 167) -> +10000 AP and Linel once per press
			{
				CommonEvent& src = ce.m_events.at(119); // field-only parallel trigger (V9 == 1)
				auto& ev = ce.m_events.at(934);
				ev.m_unknown1 = src.m_unknown1; ev.m_unknown2 = src.m_unknown2;
				ev.m_name = L"FM2 cheat hotkeys";
				ev.m_commands.clear();
				auto add = [&](CT code, uInts a, uint8_t ind) { ev.m_commands.push_back(std::make_shared<C::Command>(code, a, tStrings{}, ind)); };
				add(CT::InputKey, {1600000, 4352, 167}, 0);              // F9 -> cself0 (1 while pressed)
				if (getenv("FM2_DEBUG"))
				{
					add(CT::SetVariable, {1600001, 2500000, 0, 0}, 0);   // cself1 = latch
					ev.m_commands.push_back(std::make_shared<C::CommandSpecialClasses::Picture>(CT::Picture,
						uInts{32, 99999, 0, 1, 1, 1, 255, 0, 0, 150, 0}, tStrings{L"K=\\cself[0] L=\\cself[1]"}, 0));
				}
				add(CT::VariableCondition, {1, 1600000, 167, 2}, 0);     // if cself0 == 167 (key returns its own code)
				add(CT::ChoiceCase, {1}, 0);
				add(CT::VariableCondition, {1, 2500000, 0, 2}, 1);       //   if latch == 0
				add(CT::ChoiceCase, {1}, 1);
				add(CT::SetVariable, {2000001, 10000, 0, 256}, 2);       //     AP += 10000
				add(CT::SetVariable, {2000002, 10000, 0, 256}, 2);       //     Linel += 10000
				add(CT::SetVariable, {2500000, 1, 0, 0}, 2);             //     latch = 1
				add(CT::Blank, {}, 2);
				add(CT::BranchEnd, {}, 1);
				add(CT::Blank, {}, 1);
				add(CT::ElseCase, {0}, 0);                               // else
				add(CT::SetVariable, {2500000, 0, 0, 0}, 1);             //   latch = 0
				add(CT::Blank, {}, 1);
				add(CT::BranchEnd, {}, 0);
				add(CT::Blank, {}, 0);
			}
			else { std::cerr << "variant A|B|C" << std::endl; return 1; }
			g_activeFile = "CommonEvent.dat";
			FileCoder coder(outp.wstring(), FileCoder::Mode::WRITE, ce.m_fileType, ce.m_seedIndices);
			ce.dump(coder);
			std::cout << "wrote " << outp << " variant " << variant << " with " << ce.m_events.size() << " events" << std::endl;
			return 0;
		}
	}
	catch (std::exception& e) { std::cerr << "error: " << e.what() << "\n"; return 2; }
	std::cerr << "unknown mode\n";
	return 1;
}

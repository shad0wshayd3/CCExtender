#include "BetaComment.h"

bool BetaComment::Exec(const RE::SCRIPT_PARAMETER*, RE::SCRIPT_FUNCTION::ScriptData* a_scriptData, RE::TESObjectREFR*, RE::TESObjectREFR*, RE::Script*, RE::ScriptLocals*, double&, std::uint32_t&)
{
	_file.open(*Settings::betaCommentFileName, std::ios_base::app);
	if (!_file.is_open())
	{
		CPrint("> [%s] ERROR: Failed to open output file", LONG_NAME);
		return true;
	}

	auto selectedRef = RE::Console::GetSelectedRef();
	if (!selectedRef)
	{
		CPrint("> [%s] ERROR: No selected reference", LONG_NAME);
		return true;
	}

	auto strChunk = a_scriptData->GetStringChunk();
	auto comment = strChunk->GetString();
	_ref = selectedRef.get();
	LogComment(comment);
	_ref = 0;
	_file.close();

	return true;
}

void BetaComment::Register()
{
	using Type = RE::SCRIPT_PARAM_TYPE;

	Init();

	auto info = RE::SCRIPT_FUNCTION::LocateConsoleCommand("BetaComment");  // Unused
	if (info)
	{
		static RE::SCRIPT_PARAMETER params[] = {
			{"String", Type::kChar, 0}
		};

		info->functionName = LONG_NAME;
		info->shortName = SHORT_NAME;
		info->helpString = HelpStr();
		info->referenceFunction = false;
		info->SetParameters(params);
		info->executeFunction = Exec;
		info->conditionFunction = nullptr;
		info->editorFilter = false;
		info->invalidatesCellList = false;

		SKSE::log::info("Registered console command: {} ({})"sv, LONG_NAME, SHORT_NAME);
	}
	else
	{
		SKSE::log::error("Failed to register console command: {} ({})"sv, LONG_NAME, SHORT_NAME);
	}
}

void BetaComment::CPrint(const char* a_fmt, ...)
{
	auto console = RE::ConsoleLog::GetSingleton();
	if (console && console->IsConsoleMode())
	{
		std::va_list args;
		va_start(args, a_fmt);
		console->VPrint(a_fmt, args);
		va_end(args);
	}
}

const char* BetaComment::GetFormEditorID(RE::TESForm* a_form)
{
	switch (a_form->GetFormType())
	{
	case RE::FormType::Keyword:
	case RE::FormType::LocationRefType:
	case RE::FormType::Action:
	case RE::FormType::MenuIcon:
	case RE::FormType::Global:
	case RE::FormType::HeadPart:
	case RE::FormType::Race:
	case RE::FormType::Sound:
	case RE::FormType::Script:
	case RE::FormType::Navigation:
	case RE::FormType::Cell:
	case RE::FormType::WorldSpace:
	case RE::FormType::Land:
	case RE::FormType::NavMesh:
	case RE::FormType::Dialogue:
	case RE::FormType::Quest:
	case RE::FormType::Idle:
	case RE::FormType::AnimatedObject:
	case RE::FormType::ImageAdapter:
	case RE::FormType::VoiceType:
	case RE::FormType::Ragdoll:
	case RE::FormType::DefaultObject:
	case RE::FormType::MusicType:
	case RE::FormType::StoryManagerBranchNode:
	case RE::FormType::StoryManagerQuestNode:
	case RE::FormType::StoryManagerEventNode:
	case RE::FormType::SoundRecord:
		break;

	default:
	{
		auto hndl = REX::W32::GetModuleHandleA("po3_Tweaks");
		auto func = reinterpret_cast<const char* (*)(std::uint32_t)>(REX::W32::GetProcAddress(hndl, "GetFormEditorID"));
		if (func)
		{
			return func(a_form->formID);
		}
	}
	}

	return a_form->GetFormEditorID();
}

const char* BetaComment::HelpStr()
{
	static const std::string help = []()
	{
		std::string str;
		str += "<betacomment> \" \" <comment>";
		str += "\n\t<betacomment> ::= \"BetaComment\" | \"BC\"";
		str += "\n\t<comment> ::= <string> ; The comment string";
		return str;
	}();

	return help.c_str();
}

void BetaComment::Init()
{
	constexpr auto USERNAME_SIZE = std::extent<decltype(_userName)>::value;

	//	DWORD tmpSize = USERNAME_SIZE;
	//	if (!REX::W32::GetUserNameA(_userName, &tmpSize))
	//	{
	//		SKSE::log::error("Failed to get username with error code ({})"sv, GetLastError());
	strcpy_s(_userName, USERNAME_SIZE, "SEE-LOG-FOR-ERROR");
	//	}
}

void BetaComment::LogComment(const std::string& a_comment)
{
	Buffer line;
	Buffer tmp;

	if (!PrintUserName(line))
	{
		CPrint("> [%s] ERROR: Failed to get username", LONG_NAME);
		return;
	}
	line << ':' << _DELIM;

	line << '(';
	if (!PrintTime(line))
	{
		CPrint("> [%s] ERROR: Failed to get local time", LONG_NAME);
		return;
	}
	line << ')' << _DELIM;

	if (!PrintSourceFile(line))
	{
		line << "NO-SOURCE";
	}
	line << _DELIM;

	line << '[';
	if (!PrintCellFormID(line))
	{
		CPrint("> [%s] ERROR: Failed to get cell form ID", LONG_NAME);
		return;
	}

	if (PrintCellEditorID(tmp))
	{
		line << ": " << tmp.str();
		tmp.str("");
	}

	if (PrintCellCoordinates(tmp))
	{
		line << " (" << tmp.str() << ')';
		tmp.str("");
	}
	line << ']' << _DELIM;

	line << '[';
	if (!PrintRefFormID(line))
	{
		CPrint("> [%s] ERROR: Failed to get selected ref form ID", LONG_NAME);
		return;
	}
	line << ": ";

	if (PrintRefEditorID(tmp))
	{
		line << tmp.str() << ' ';
		tmp.str("");
	}

	line << '(';
	if (!PrintRefCoordinates(line))
	{
		CPrint("> [%s] ERROR: Failed to get selected ref coordinates", LONG_NAME);
		return;
	}
	line << ")]" << _DELIM;

	line << '\"' << a_comment << '\"';

	_file << line.str() << '\n';
	_file.flush();
}

bool BetaComment::PrintCellCoordinates(Buffer& a_buf)
{
	auto cell = _ref->GetParentCell();
	if (!cell)
	{
		return false;
	}

	auto coordinates = cell->GetCoordinates();
	if (!coordinates)
	{
		return false;
	}

	a_buf << coordinates->cellX << ", " << coordinates->cellY;

	return true;
}

bool BetaComment::PrintCellEditorID(Buffer& a_buf)
{
	auto cell = _ref->GetParentCell();
	if (!cell)
	{
		return false;
	}

	std::string editorID{ stl::safe_string(GetFormEditorID(cell)) };
	if (editorID.empty())
	{
		return false;
	}

	a_buf << editorID;

	return true;
}

bool BetaComment::PrintCellFormID(Buffer& a_buf)
{
	char formID[] = "0xDEADBEEF";
	auto cell = _ref->GetParentCell();
	std::snprintf(formID, sizeof(formID), "0x%08X", cell->formID);
	a_buf << formID;
	return true;
}

bool BetaComment::PrintRefCoordinates(Buffer& a_buf)
{
	a_buf << std::floor(_ref->GetPositionX()) << ", " << std::floor(_ref->GetPositionY()) << ", " << std::floor(_ref->GetPositionZ());
	return true;
}

bool BetaComment::PrintRefEditorID(Buffer& a_buf)
{
	std::string editorID{ stl::safe_string(GetFormEditorID(_ref)) };
	if (editorID.empty())
	{
		return false;
	}

	a_buf << editorID;

	return true;
}

bool BetaComment::PrintRefFormID(Buffer& a_buf)
{
	char formID[] = "0xDEADBEEF";
	std::snprintf(formID, sizeof(formID), "0x%08X", _ref->formID);
	a_buf << formID;
	return true;
}

bool BetaComment::PrintSourceFile(Buffer& a_buf)
{
	auto source = _ref->GetDescriptionOwnerFile();
	if (!source)
	{
		return false;
	}

	a_buf << source->fileName;

	return true;
}

bool BetaComment::PrintTime(Buffer& a_buf)
{
	auto time = std::time(nullptr);
	std::tm localTime;
	auto err = gmtime_s(&localTime, &time);
	if (err)
	{
		return false;
	}

	a_buf << std::put_time(&localTime, "%F %T GMT");

	return true;
}

bool BetaComment::PrintUserName(Buffer& a_buf)
{
	a_buf << _userName;
	return true;
}

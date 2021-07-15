#include "filezilla.h"
#include "Options.h"
#include "filezillaapp.h"
#include "file_utils.h"
#include "locale_initializer.h"
#include "sizeformatting.h"
#include "xmlfunctions.h"

#include "../commonui/fz_paths.h"
#include "../commonui/ipcmutex.h"

#include <libfilezilla/local_filesys.hpp>
#include <libfilezilla/util.hpp>

#include <algorithm>
#include <string>

#include <stdlib.h>

#ifdef FZ_WINDOWS
	#include <shlobj.h>

	// Needed for MinGW:
	#ifndef SHGFP_TYPE_CURRENT
		#define SHGFP_TYPE_CURRENT 0
	#endif
#endif

COptions* COptions::m_theOptions = 0;

#ifdef FZ_WINDOWS
//case insensitive
#define DEFAULT_FILENAME_SORT   0
#else
//case sensitive
#define DEFAULT_FILENAME_SORT   1
#endif

namespace {
#ifdef FZ_WINDOWS
	auto const platform_name = "win";
#elif defined(FZ_MAC)
	auto const platform_name = "mac";
#else
	auto const platform_name = "unix";
#endif

option_registrator r(&register_interface_options);
std::string const product_name{};
}
unsigned int register_interface_options()
{
	// Note: A few options are versioned due to a changed
	// option syntax or past, unhealthy defaults
	static int const value = register_options({
		// Default/internal options
		{ "Config Location", L"", option_flags::predefined_only | option_flags::platform },
		{ "Kiosk mode", 0, option_flags::predefined_priority, 0, 2 },
		{ "Cache directory", L"", option_flags::predefined_priority | option_flags::platform },

		// Normal UI options
		{ "Number of Transfers", 2, option_flags::numeric_clamp, 1, 10 },
		{ "Ascii Binary mode", 0, option_flags::normal, 0, 2 },
		{ "Auto Ascii files", L"ac|am|asp|bat|c|cfm|cgi|conf|cpp|css|dhtml|diff|diz|h|hpp|htm|html|in|inc|java|js|jsp|lua|m4|mak|md5|nfo|nsh|nsi|pas|patch|pem|php|phtml|pl|po|pot|py|qmail|sh|sha1|sha256|sha512|shtml|sql|svg|tcl|tpl|txt|vbs|xhtml|xml|xrc", option_flags::normal },
		{ "Auto Ascii no extension", L"1", option_flags::normal },
		{ "Auto Ascii dotfiles", true, option_flags::normal },
		{ "Language Code", L"", option_flags::normal, 50 },
		{ "Concurrent download limit", 0, option_flags::numeric_clamp, 0, 10 },
		{ "Concurrent upload limit", 0, option_flags::numeric_clamp, 0, 10 },
		{ "Show debug menu", false, option_flags::normal },
		{ "File exists action download", 0, option_flags::normal, 0, 7 },
		{ "File exists action upload", 0, option_flags::normal, 0, 7 },
		{ "Allow ascii resume", false, option_flags::normal },
		{ "Greeting version", L"", option_flags::normal },
		{ "Greeting resources", L"", option_flags::normal },
		{ "Onetime Dialogs", L"", option_flags::normal },
		{ "Show Tree Local", true, option_flags::normal },
		{ "Show Tree Remote", true, option_flags::normal },
		{ "File Pane Layout", 0, option_flags::normal, 0, 3 },
		{ "File Pane Swap", false, option_flags::normal },
		{ "Filelist directory sort", 0, option_flags::normal, 0, 2 },
		{ "Filelist name sort", DEFAULT_FILENAME_SORT, option_flags::normal, 0, 2 },
		{ "Queue successful autoclear", false, option_flags::normal },
		{ "Queue column widths", L"", option_flags::normal },
		{ "Local filelist colwidths", L"", option_flags::normal },
		{ "Remote filelist colwidths", L"", option_flags::normal },
		{ "Window position and size", L"", option_flags::normal },
		{ "Splitter positions (v2)", L"", option_flags::normal },
		{ "Local filelist sortorder", L"", option_flags::normal },
		{ "Remote filelist sortorder", L"", option_flags::normal },
		{ "Time Format", L"", option_flags::normal },
		{ "Date Format", L"", option_flags::normal },
		{ "Show message log", true, option_flags::normal },
		{ "Show queue", true, option_flags::normal },
		{ "Default editor", L"", option_flags::platform },
		{ "Always use default editor", true, option_flags::normal },
		{ "File associations (v2)", L"", option_flags::platform },
		{ "Comparison mode", 1, option_flags::normal, 0, 1 },
		{ "Comparison threshold", 1, option_flags::normal, 0, 1440 },
		{ "Site Manager position", L"", option_flags::normal },
		{ "Icon theme", L"default", option_flags::normal },
		{ "Icon scale", 125, option_flags::numeric_clamp, 25, 400 },
		{ "Timestamp in message log", false, option_flags::normal },
		{ "Sitemanager last selected", L"", option_flags::normal },
		{ "Local filelist shown columns", L"", option_flags::normal },
		{ "Remote filelist shown columns", L"", option_flags::normal },
		{ "Local filelist column order", L"", option_flags::normal },
		{ "Remote filelist column order", L"", option_flags::normal },
		{ "Filelist status bar", true, option_flags::normal },
		{ "Filter toggle state", false, option_flags::normal },
		{ "Show quickconnect bar", true, option_flags::normal },
		{ "Messagelog position", 0, option_flags::normal, 0, 2 },
		{ "File doubleclick action", 0, option_flags::normal, 0, 3 },
		{ "Dir doubleclick action", 0, option_flags::normal, 0, 3 },
		{ "Minimize to tray", false, option_flags::normal },
		{ "Search column widths", L"", option_flags::normal },
		{ "Search column shown", L"", option_flags::normal },
		{ "Search column order", L"", option_flags::normal },
		{ "Search window size", L"", option_flags::normal },
		{ "Comparison hide identical", false, option_flags::normal },
		{ "Search sort order", L"", option_flags::normal },
		{ "Edit track local", true, option_flags::normal },
		{ "Prevent idle sleep", true, option_flags::normal },
		{ "Filteredit window size", L"", option_flags::normal },
		{ "Enable invalid char filter", true, option_flags::normal },
		{ "Invalid char replace", L"_", option_flags::normal, option_type::string, 1, [](std::wstring& v) {
			return v.size() == 1 && !IsInvalidChar(v[0], true);
		}},
		{ "Already connected choice", 0, option_flags::normal },
		{ "Edit status dialog size", L"", option_flags::normal },
		{ "Display current speed", false, option_flags::normal },
		{ "Toolbar hidden", false, option_flags::normal },
		{ "Strip VMS revisions", false, option_flags::normal },
		{ "Startup action", 0, option_flags::normal },
		{ "Prompt password save", 0, option_flags::normal },
		{ "Persistent Choices", 0, option_flags::normal },
		{ "Queue completion action", 1, option_flags::normal },
		{ "Queue completion command", L"", option_flags::normal },
		{ "Drag and Drop disabled", false, option_flags::normal },
		{ "Disable update footer", false, option_flags::normal },
		{ "Master password encryptor", L"", option_flags::normal },
		{ "Tab data", L"", option_flags::normal | option_flags::sensitive_data, option_type::xml },
		{ "Highest shown overlay id", 0, option_flags::normal }
	});
	return value;
}

optionsIndex mapOption(interfaceOptions opt)
{
	static unsigned int const offset = register_interface_options();

	auto ret = optionsIndex::invalid;
	if (opt < OPTIONS_NUM) {
		return static_cast<optionsIndex>(opt + offset);
	}
	return ret;
}

BEGIN_EVENT_TABLE(COptions, wxEvtHandler)
EVT_TIMER(wxID_ANY, COptions::OnTimer)
END_EVENT_TABLE()

COptions::COptions()
{
	m_theOptions = this;

	m_save_timer.SetOwner(this);

	LoadGlobalDefaultOptions();

	CLocalPath const dir = InitSettingsDir();

	CInterProcessMutex mutex(MUTEX_OPTIONS);
	xmlFile_ = std::make_unique<CXmlFile>(dir.GetPath() + L"filezilla.xml");
	if (!xmlFile_->Load()) {
		wxString msg = xmlFile_->GetError() + L"\n\n" + _("For this session the default settings will be used. Any changes to the settings will not be saved.");
		wxMessageBoxEx(msg, _("Error loading xml file"), wxICON_ERROR);
		xmlFile_.reset();
	}
	else {
		auto settings = CreateSettingsXmlElement();
		Load(settings, false, false);
	}

	{
		fz::scoped_write_lock l(mtx_);
		changed_.clear();
		can_notify_ = true;
	}

	if (dirty_ && !m_save_timer.IsRunning()) {
		m_save_timer.Start(15000, true);
	}
}

COptions::~COptions()
{
}

pugi::xml_node COptions::CreateSettingsXmlElement()
{
	if (!xmlFile_) {
		return pugi::xml_node();
	}

	auto element = xmlFile_->GetElement();
	if (!element) {
		return element;
	}

	auto settings = element.child("Settings");
	if (!settings) {
		settings = element.append_child("Settings");
	}

	return settings;
}

void COptions::Init()
{
	if (!m_theOptions) {
		new COptions(); // It sets m_theOptions internally itself
	}
}

void COptions::Destroy()
{
	if (!m_theOptions) {
		return;
	}

	delete m_theOptions;
	m_theOptions = 0;
}

COptions* COptions::Get()
{
	return m_theOptions;
}

void COptions::Import(pugi::xml_node & element)
{
	Load(element, false, true);
}

void COptions::Load(pugi::xml_node & settings, bool predefined, bool importing)
{
	if (!settings) {
		return;
	}

	fz::scoped_write_lock l(mtx_);
	add_missing(l);

	std::vector<uint8_t> seen;
	seen.resize(options_.size());

	pugi::xml_node next;
	for (auto setting = settings.child("Setting"); setting; setting = next) {
		next = setting.next_sibling("Setting");

		const char* name = setting.attribute("name").value();
		if (!name) {
			continue;
		}

		auto def_it = name_to_option_.find(name);
		if (def_it == name_to_option_.cend()) {
			continue;
		}

		auto const& def = options_[def_it->second];

		if (def.flags() & option_flags::platform) {
			char const* p = setting.attribute("platform").value();
			if (*p && strcmp(p, platform_name)) {
				continue;
			}
		}
		if (def.flags() & option_flags::product) {
			char const* p = setting.attribute("product").value();
			if (product_name != p) {
				continue;
			}
		}

		if (seen[def_it->second]) {
			if (!predefined && !importing) {
				settings.remove_child(setting);
				dirty_ = true;
			}
			continue;
		}
		seen[def_it->second] = 1;

		auto & val = values_[def_it->second];

		switch (def.type()) {
		case option_type::number:
		case option_type::boolean:
			set(static_cast<optionsIndex>(def_it->second), def, val, setting.text().as_int(), predefined);
			break;
		case option_type::xml:
			{
				pugi::xml_document doc;
				for (auto c = setting.first_child(); c; c = c.next_sibling()) {
					doc.append_copy(c);
				}
				set(static_cast<optionsIndex>(def_it->second), def, val, std::move(doc), predefined);
			}
			break;
		default:
			set(static_cast<optionsIndex>(def_it->second), def, val, fz::to_wstring_from_utf8(setting.child_value()), predefined);
		}
	}

	if (!predefined && !importing) {
		for (size_t i = 0; i < seen.size(); ++i) {
			if (seen[i]) {
				continue;
			}

			set_xml_value(settings, i, false);
		}
	}
}

void COptions::LoadGlobalDefaultOptions()
{
	CLocalPath const defaultsDir = GetDefaultsDir();
	if (defaultsDir.empty()) {
		return;
	}
	CXmlFile file(defaultsDir.GetPath() + L"fzdefaults.xml");
	if (!file.Load()) {
		return;
	}

	auto element = file.GetElement();
	if (!element) {
		return;
	}

	element = element.child("Settings");
	if (!element) {
		return;
	}

	Load(element, true, false);
}

void COptions::OnTimer(wxTimerEvent&)
{
	Save(false);
}

void COptions::Save(bool processChanged)
{
	if (processChanged) {
		continue_notify_changed();
	}

	m_save_timer.Stop();
	if (!dirty_) {
		return;
	}
	dirty_ = false;

	if (get_int(OPTION_DEFAULT_KIOSKMODE) == 2) {
		return;
	}

	if (!xmlFile_) {
		return;
	}

	CInterProcessMutex mutex(MUTEX_OPTIONS);
	SaveWithErrorDialog(*xmlFile_);
}

bool COptions::Cleanup()
{
	bool ret = false;

	fz::scoped_write_lock l(mtx_);

	// Clear known sensitive settings
	for (size_t i = 0; i < options_.size(); ++i) {
		if (options_[i].flags() & option_flags::sensitive_data) {
			set_default_value(static_cast<optionsIndex>(i));
			set_changed(static_cast<optionsIndex>(i));
		}
	}

	auto element = xmlFile_->GetElement();
	auto settings = element.child("Settings");

	// Remove all but the first settings element
	auto child = settings.next_sibling("Settings");
	while (child) {
		auto next = child.next_sibling("Settings");
		element.remove_child(child);
		child = next;
	}

	child = settings.first_child();

	while (child) {
		auto next = child.next_sibling();

		if (child.name() == std::string("Setting")) {
			// Remove sensitive settings
			if (!strcmp(child.attribute("sensitive").value(), "1")) {
				settings.remove_child(child);
				ret = true;
			}
		}
		else {
			// Only settings in the settings node.
			settings.remove_child(child);
			ret = true;
		}
		child = next;
	}

	if (ret) {
		dirty_ = true;
		if (!m_save_timer.IsRunning()) {
			m_save_timer.Start(15000, true);
		}
	}

	return ret;
}

namespace {
#ifndef FZ_WINDOWS
std::wstring TryDirectory(wxString path, wxString const& suffix, bool check_exists)
{
	if (!path.empty() && path[0] == '/') {
		if (path[path.size() - 1] != '/') {
			path += '/';
		}

		path += suffix;

		if (check_exists) {
			if (!wxFileName::DirExists(path)) {
				path.clear();
			}
		}
	}
	else {
		path.clear();
	}
	return path.ToStdWstring();
}

#endif
}

CLocalPath COptions::GetUnadjustedSettingsDir() {
	return ::GetUnadjustedSettingsDir();
}

CLocalPath COptions::GetCacheDirectory()
{
	CLocalPath ret;

	std::wstring dir(get_string(OPTION_DEFAULT_CACHE_DIR));
	if (!dir.empty()) {
		dir = ExpandPath(dir);
		ret.SetPath(GetDefaultsDir().GetPath());
		if (!ret.ChangePath(dir)) {
			ret.clear();
		}
	}
	else {
#ifdef FZ_WINDOWS
		wchar_t* out{};
		if (SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, 0, &out) == S_OK) {
			ret.SetPath(out);
			CoTaskMemFree(out);
		}
		if (!ret.empty()) {
			ret.AddSegment(L"FileZilla");
		}
#else
		std::wstring cfg = TryDirectory(GetEnv("XDG_CACHE_HOME"), L"filezilla/", false);
		if (cfg.empty()) {
			cfg = TryDirectory(wxGetHomeDir(), L".cache/filezilla/", false);
		}
		ret.SetPath(cfg);
#endif
	}

	return ret;
}

CLocalPath COptions::InitSettingsDir()
{
	CLocalPath p;

	std::wstring dir = get_string(OPTION_DEFAULT_SETTINGSDIR);
	if (!dir.empty()) {
		dir = ExpandPath(dir);
		p.SetPath(GetDefaultsDir().GetPath());
		p.ChangePath(dir);
	}
	else {
		p = GetUnadjustedSettingsDir();
	}

	if (!p.empty() && !p.Exists()) {
		fz::mkdir(fz::to_native(p.GetPath()), true, fz::mkdir_permissions::cur_user);
	}

	set(mapOption(OPTION_DEFAULT_SETTINGSDIR), p.GetPath(), true);
	set_ipcmutex_lockfile_path(p.GetPath());

	return p;
}

void COptions::process_changed(watched_options const& changed)
{
	auto settings = CreateSettingsXmlElement();
	if (!settings) {
		return;
	}
	for (size_t i = 0; i < changed.options_.size(); ++i) {
		uint64_t v = changed.options_[i];
		while (v) {
			auto bit = fz::bitscan(v);
			v ^= 1ull << bit;
			size_t opt = bit + i * 64;

			set_xml_value(settings, opt, true);
		}
	}

	if (dirty_ && !m_save_timer.IsRunning()) {
		m_save_timer.Start(15000, true);
	}
}

void COptions::set_xml_value(pugi::xml_node & settings, size_t opt, bool clean)
{
	auto const& def = options_[opt];
	if (def.flags() & (option_flags::internal | option_flags::predefined_only)) {
		return;
	}

	if (clean) {
		for (pugi::xml_node it = settings.child("Setting"); it;) {
			auto cur = it;
			it = it.next_sibling("Setting");

			char const* attribute = cur.attribute("name").value();
			if (strcmp(attribute, def.name().c_str())) {
				continue;
			}

			if (def.flags() & option_flags::platform) {
				// Ignore items from the wrong platform
				char const* p = cur.attribute("platform").value();
				if (*p && strcmp(p, platform_name)) {
					continue;
				}
			}

			if (def.flags() & option_flags::product) {
				// Ignore items from the wrong product
				char const* p = cur.attribute("product").value();
				if (product_name != p) {
					continue;
				}
			}

			settings.remove_child(cur);
		}
	}

	auto setting = settings.append_child("Setting");
	setting.append_attribute("name").set_value(def.name().c_str());
	if (def.flags() & option_flags::platform) {
		setting.append_attribute("platform").set_value(platform_name);
	}
	if (def.flags() & option_flags::product && !product_name.empty()) {
		setting.append_attribute("product").set_value(product_name.c_str());
	}

	if (def.flags() & option_flags::sensitive_data) {
		setting.append_attribute("sensitive").set_value("1");
	}

	auto const& val = values_[opt];
	if (def.type() == option_type::xml) {
		for (auto c = val.xml_->first_child(); c; c = c.next_sibling()) {
			setting.append_copy(c);
		}
	}
	else {
		setting.text().set(fz::to_utf8(val.str_).c_str());
	}

	dirty_ = true;
}

void COptions::notify_changed()
{
	CallAfter([this](){ continue_notify_changed(); });
}

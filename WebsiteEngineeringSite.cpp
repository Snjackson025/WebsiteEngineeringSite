// WebsiteEngineeringSite.cpp
// Full-scale website and program builder.
// A customer describes the site or program they need, the Architect
// character turns the description into a brief, and the builder engines
// generate the real site files and program scaffolds to suit.
// NOTE: _CRT_SECURE_NO_WARNINGS must come before the includes: MSVC
// rejects std::localtime (C4996) without it, and this file timestamps
// the project memory and event log with localtime.
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>
#include <chrono>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <cctype>

// ======= CORE INFRASTRUCTURE =======
namespace Core {
    struct DataPacket {
        std::string id;
        std::string sourceEngine;
        std::map<std::string, std::string> metadata;
    };

    class Engine {
    protected:
        std::string engineName;
        bool isActive;
    public:
        Engine(const std::string& name) : engineName(name), isActive(false) {}
        virtual ~Engine() = default;
        virtual void initialize() = 0;
        virtual void shutdown() = 0;
        virtual DataPacket process(const DataPacket& input) = 0;
        std::string getName() const { return engineName; }
    };

    class EngineRegistry {
        std::vector<std::shared_ptr<Engine>> engines;
    public:
        void registerEngine(std::shared_ptr<Engine> engine) { engines.push_back(engine); }
        void startAll() { for (auto& e : engines) e->initialize(); }
        void stopAll() { for (auto& e : engines) e->shutdown(); }
    };
}

// ======= HOME MODULE =======
namespace HomeModule {
    class HomeEngine : public Core::Engine {
    public:
        HomeEngine() : Core::Engine("HomeEngine") {}
        void initialize() override { isActive = true; std::cout << "[HomeEngine] Initialized\n"; }
        void shutdown() override { isActive = false; std::cout << "[HomeEngine] Shutdown\n"; }
        Core::DataPacket process(const Core::DataPacket& input) override {
            (void)input;
            return {};
        }
    };
}

// ======= CUSTOMER BRIEF =======
// What the customer described, in a structured form the builders can use.
struct CustomerBrief {
    std::string businessName;
    std::string siteType;   // Business, Portfolio, Shop, Blog
    std::string tagline;
    std::string theme;      // warm, modern, dark, fresh
    std::vector<std::string> pages;
    std::vector<std::string> features;

    bool isComplete() const {
        return !businessName.empty() && !siteType.empty() && !pages.empty();
    }

    std::string slug() const {
        std::string s;
        for (char c : businessName) {
            if (c == ' ') s += '_';
            else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
                s += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        if (s.empty()) s = "site";
        return s;
    }
};

// ======= TEMPLATE LIBRARY =======
// Page sets and colour themes for each kind of site.
struct TemplateLibrary {
    static std::vector<std::string> pagesFor(const std::string& siteType) {
        if (siteType == "Shop") return {"Home", "Shop", "About", "Contact"};
        if (siteType == "Portfolio") return {"Home", "Work", "About", "Contact"};
        if (siteType == "Blog") return {"Home", "Articles", "About", "Contact"};
        return {"Home", "About", "Services", "Contact"};  // Business default
    }

    // theme -> {primary, background, accent}
    static std::map<std::string, std::string> colorsFor(const std::string& theme) {
        if (theme == "modern") return {{"primary", "#2563eb"}, {"background", "#f8fafc"}, {"accent", "#0ea5e9"}};
        if (theme == "dark") return {{"primary", "#f59e0b"}, {"background", "#111827"}, {"accent", "#fbbf24"}};
        if (theme == "fresh") return {{"primary", "#059669"}, {"background", "#f0fdf4"}, {"accent", "#34d399"}};
        return {{"primary", "#b45309"}, {"background", "#fffbeb"}, {"accent", "#d97706"}};  // warm default
    }
};

// ======= BRIEF ENGINE =======
// Turns a customer's words into a structured brief.
namespace BriefModule {
    class BriefEngine : public Core::Engine {
        CustomerBrief brief;
        bool hasBrief = false;
    public:
        BriefEngine() : Core::Engine("BriefEngine") {}
        void initialize() override { isActive = true; std::cout << "[BriefEngine] Initialized\n"; }
        void shutdown() override { isActive = false; std::cout << "[BriefEngine] Shutdown\n"; }
        Core::DataPacket process(const Core::DataPacket& input) override {
            (void)input;
            return {};
        }

        void createBrief(const CustomerBrief& b) {
            brief = b;
            if (brief.pages.empty()) brief.pages = TemplateLibrary::pagesFor(brief.siteType);
            if (brief.theme.empty()) brief.theme = "warm";
            hasBrief = true;
            std::cout << "[BriefEngine] Brief recorded for '" << brief.businessName << "' (" << brief.siteType << ")\n";
        }

        bool hasCurrentBrief() const { return hasBrief; }
        CustomerBrief getBrief() const { return brief; }

        void showBrief() const {
            if (!hasBrief) { std::cout << "No brief yet. Describe a site first.\n"; return; }
            std::cout << "--- CUSTOMER BRIEF ---\n";
            std::cout << "Business: " << brief.businessName << "\n";
            std::cout << "Type: " << brief.siteType << "\n";
            std::cout << "Tagline: " << brief.tagline << "\n";
            std::cout << "Theme: " << brief.theme << "\n";
            std::cout << "Pages:";
            for (const auto& p : brief.pages) std::cout << " " << p;
            std::cout << "\nFeatures:";
            for (const auto& f : brief.features) std::cout << " " << f;
            std::cout << "\n";
        }
    };
}

// ======= SITE BUILDER ENGINE =======
// Turns a brief into real website files: one HTML page per page in the brief.
namespace SiteBuilderModule {
    struct BuildReport {
        bool ok = false;
        std::vector<std::string> files;
        std::string summary;
    };

    class SiteBuilderEngine : public Core::Engine {
    public:
        SiteBuilderEngine() : Core::Engine("SiteBuilderEngine") {}
        void initialize() override { isActive = true; std::cout << "[SiteBuilderEngine] Initialized\n"; }
        void shutdown() override { isActive = false; std::cout << "[SiteBuilderEngine] Shutdown\n"; }
        Core::DataPacket process(const Core::DataPacket& input) override {
            (void)input;
            return {};
        }

        std::string pageHtml(const CustomerBrief& brief, const std::string& page) const {
            auto colors = TemplateLibrary::colorsFor(brief.theme);
            std::ostringstream nav;
            for (const auto& p : brief.pages) {
                nav << "<a href=\"" << brief.slug() << "_" << lower(p) << ".html\">" << p << "</a> ";
            }
            std::ostringstream features;
            for (const auto& f : brief.features) features << "<li>" << f << "</li>\n";
            std::ostringstream html;
            html << "<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"utf-8\">\n"
                 << "<title>" << brief.businessName << " - " << page << "</title>\n"
                 << "<style>body{font-family:sans-serif;background:" << colors["background"]
                 << ";color:#1f2937;margin:0}header{background:" << colors["primary"]
                 << ";color:#fff;padding:24px}header a{color:#fff;margin-right:16px}"
                 << "main{padding:24px}.tag{color:" << colors["accent"] << "}</style>\n</head>\n<body>\n"
                 << "<header><h1>" << brief.businessName << "</h1><p class=\"tag\">" << brief.tagline << "</p><nav>"
                 << nav.str() << "</nav></header>\n"
                 << "<main><h2>" << page << "</h2>\n"
                 << "<p>Welcome to " << brief.businessName << " — " << brief.tagline << "</p>\n";
            if (!brief.features.empty()) html << "<h3>What we offer</h3>\n<ul>\n" << features.str() << "</ul>\n";
            html << "<p><a href=\"" << brief.slug() << "_contact.html\">Get in touch</a></p>\n"
                 << "</main>\n<footer><p>Built for " << brief.businessName << " by WebsiteEngineeringSite</p></footer>\n"
                 << "</body>\n</html>\n";
            return html.str();
        }

        BuildReport buildSite(const CustomerBrief& brief) {
            BuildReport report;
            if (!brief.isComplete()) {
                report.summary = "Cannot build: the brief is incomplete.";
                return report;
            }
            for (const auto& page : brief.pages) {
                std::string filename = brief.slug() + "_" + lower(page) + ".html";
                std::ofstream out(filename);
                if (!out) {
                    report.summary = "Cannot build: could not write " + filename;
                    return report;
                }
                out << pageHtml(brief, page);
                report.files.push_back(filename);
            }
            report.ok = true;
            std::ostringstream s;
            s << "Site built for '" << brief.businessName << "': " << report.files.size() << " pages";
            report.summary = s.str();
            std::cout << "[SiteBuilderEngine] " << report.summary << "\n";
            for (const auto& f : report.files) std::cout << "  wrote " << f << "\n";
            return report;
        }

    private:
        static std::string lower(const std::string& s) {
            std::string r = s;
            std::transform(r.begin(), r.end(), r.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            return r;
        }
    };
}

// ======= PROGRAM BUILDER ENGINE =======
// Turns a brief into a program scaffold the customer can build on.
namespace ProgramBuilderModule {
    struct ProgramReport {
        bool ok = false;
        std::vector<std::string> files;
        std::string summary;
    };

    class ProgramBuilderEngine : public Core::Engine {
    public:
        ProgramBuilderEngine() : Core::Engine("ProgramBuilderEngine") {}
        void initialize() override { isActive = true; std::cout << "[ProgramBuilderEngine] Initialized\n"; }
        void shutdown() override { isActive = false; std::cout << "[ProgramBuilderEngine] Shutdown\n"; }
        Core::DataPacket process(const Core::DataPacket& input) override {
            (void)input;
            return {};
        }

        ProgramReport buildProgram(const CustomerBrief& brief) {
            ProgramReport report;
            if (!brief.isComplete()) {
                report.summary = "Cannot build: the brief is incomplete.";
                return report;
            }
            std::string base = brief.slug() + "_program";
            std::string cppFile = base + ".cpp";
            std::string readmeFile = base + "_README.txt";
            std::ofstream cpp(cppFile);
            std::ofstream readme(readmeFile);
            if (!cpp || !readme) {
                report.summary = "Cannot build: could not write program files.";
                return report;
            }
            cpp << "// " << brief.businessName << " — generated program scaffold\n"
                << "// Type: " << brief.siteType << ". Built by WebsiteEngineeringSite.\n"
                << "#include <iostream>\n\n"
                << "int main() {\n"
                << "    std::cout << \"Welcome to " << brief.businessName << "!\\n\";\n"
                << "    std::cout << \"" << brief.tagline << "\\n\";\n"
                << "    return 0;\n}\n";
            readme << brief.businessName << "\n"
                   << "Type: " << brief.siteType << "\n"
                   << "Tagline: " << brief.tagline << "\n"
                   << "Features:";
            for (const auto& f : brief.features) readme << " " << f;
            readme << "\nBuild: g++ -std=c++17 -o " << base << " " << cppFile << "\n";
            report.files.push_back(cppFile);
            report.files.push_back(readmeFile);
            report.ok = true;
            std::ostringstream s;
            s << "Program built for '" << brief.businessName << "': " << report.files.size() << " files";
            report.summary = s.str();
            std::cout << "[ProgramBuilderEngine] " << report.summary << "\n";
            for (const auto& f : report.files) std::cout << "  wrote " << f << "\n";
            return report;
        }
    };
}

// ======= ARCHITECT CHARACTER =======
// The AI character who talks to the customer, confirms the brief,
// and presents the finished site or program.
class ArchitectCharacter {
    std::string name = "Architect";
public:
    std::string getName() const { return name; }
    void setName(const std::string& n) { if (!n.empty()) name = n; }

    void greet() const {
        std::cout << "[" << name << "] Hello! Tell me about the website or program you need,\n"
                  << "and I will build it to suit you.\n";
    }

    void confirmBrief(const CustomerBrief& brief) const {
        std::cout << "[" << name << "] Got it — a " << brief.siteType << " site for '"
                  << brief.businessName << "', " << brief.pages.size() << " pages, "
                  << brief.theme << " theme. I will build that for you.\n";
    }

    void presentSite(const SiteBuilderModule::BuildReport& report) const {
        if (!report.ok) { std::cout << "[" << name << "] " << report.summary << "\n"; return; }
        std::cout << "[" << name << "] Your site is ready: " << report.summary << ".\n"
                  << "Open the first page in a browser to see it.\n";
    }

    void presentProgram(const ProgramBuilderModule::ProgramReport& report) const {
        if (!report.ok) { std::cout << "[" << name << "] " << report.summary << "\n"; return; }
        std::cout << "[" << name << "] Your program is ready: " << report.summary << ".\n";
    }

    void respond(const std::string& text) const {
        std::string lower = text;
        std::transform(lower.begin(), lower.end(), lower.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (lower.find("site") != std::string::npos || lower.find("website") != std::string::npos)
            std::cout << "[" << name << "] I build websites — tell me the business name, the kind of site, and the pages you want.\n";
        else if (lower.find("program") != std::string::npos || lower.find("app") != std::string::npos)
            std::cout << "[" << name << "] I build programs too — tell me what it should do and I will scaffold it for you.\n";
        else if (lower.find("price") != std::string::npos || lower.find("cost") != std::string::npos)
            std::cout << "[" << name << "] Every build is shaped to the brief — tell me what you need and I will scope it.\n";
        else if (lower.find("help") != std::string::npos)
            std::cout << "[" << name << "] Describe a site or a program and I will build it. Try option 1 in the menu to start a brief.\n";
        else
            std::cout << "[" << name << "] Noted. Tell me more about the site or program you have in mind.\n";
    }
};

// ======= PROJECT MEMORY =======
// Persistent record of every site and program built for customers.
class ProjectMemory {
    std::string filename = "website_projects.txt";
public:
    void record(const std::string& kind, const CustomerBrief& brief, const std::vector<std::string>& files) {
        std::ofstream out(filename, std::ios::app);
        if (!out) return;
        std::time_t now = std::time(nullptr);
        char stamp[32];
        std::strftime(stamp, sizeof(stamp), "%Y-%m-%d %H:%M", std::localtime(&now));
        out << stamp << " | " << kind << " | " << brief.businessName << " | " << brief.siteType
            << " | " << files.size() << " files\n";
    }

    void listProjects() const {
        std::ifstream in(filename);
        if (!in) { std::cout << "No projects built yet.\n"; return; }
        std::cout << "--- BUILT PROJECTS ---\n";
        std::string line;
        bool any = false;
        while (std::getline(in, line)) { std::cout << line << "\n"; any = true; }
        if (!any) std::cout << "No projects built yet.\n";
    }
};

// ======= EVENT LOGGER =======
class EventLogger {
    std::string filename = "website_events.txt";
public:
    void log(const std::string& event) {
        std::ofstream out(filename, std::ios::app);
        if (!out) return;
        std::time_t now = std::time(nullptr);
        char stamp[32];
        std::strftime(stamp, sizeof(stamp), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
        out << stamp << " | " << event << "\n";
    }
};

// ======= CENTRAL NODE =======
class CentralNode {
    std::shared_ptr<Core::EngineRegistry> registry;
    std::shared_ptr<HomeModule::HomeEngine> home;
    std::shared_ptr<BriefModule::BriefEngine> briefEngine;
    std::shared_ptr<SiteBuilderModule::SiteBuilderEngine> siteBuilder;
    std::shared_ptr<ProgramBuilderModule::ProgramBuilderEngine> programBuilder;
    ArchitectCharacter architect;
    ProjectMemory memory;
    EventLogger logger;
public:
    CentralNode() {
        registry = std::make_shared<Core::EngineRegistry>();
        home = std::make_shared<HomeModule::HomeEngine>();
        briefEngine = std::make_shared<BriefModule::BriefEngine>();
        siteBuilder = std::make_shared<SiteBuilderModule::SiteBuilderEngine>();
        programBuilder = std::make_shared<ProgramBuilderModule::ProgramBuilderEngine>();
        registry->registerEngine(home);
        registry->registerEngine(briefEngine);
        registry->registerEngine(siteBuilder);
        registry->registerEngine(programBuilder);
    }

    void initialize() { registry->startAll(); }
    void shutdown() { registry->stopAll(); }

    void runDemo() {
        std::cout << "--- DEMO RUN ---\n";
        architect.greet();
        CustomerBrief brief;
        brief.businessName = "Rosewood Bakery";
        brief.siteType = "Business";
        brief.tagline = "Fresh bread, baked every morning.";
        brief.theme = "warm";
        brief.features = {"Daily menu", "Order form", "Opening hours"};
        briefEngine->createBrief(brief);
        briefEngine->showBrief();
        architect.confirmBrief(briefEngine->getBrief());
        auto site = siteBuilder->buildSite(briefEngine->getBrief());
        architect.presentSite(site);
        if (site.ok) {
            memory.record("site", brief, site.files);
            logger.log("Demo site built for Rosewood Bakery");
        }
        auto program = programBuilder->buildProgram(briefEngine->getBrief());
        architect.presentProgram(program);
        if (program.ok) memory.record("program", brief, program.files);
    }

    void describeSite() {
        std::string line;
        CustomerBrief brief;
        std::cout << "Business name: ";
        std::getline(std::cin, brief.businessName);
        std::cout << "Site type (Business/Portfolio/Shop/Blog): ";
        std::getline(std::cin, brief.siteType);
        if (brief.siteType.empty()) brief.siteType = "Business";
        std::cout << "Tagline: ";
        std::getline(std::cin, brief.tagline);
        std::cout << "Theme (warm/modern/dark/fresh): ";
        std::getline(std::cin, brief.theme);
        std::cout << "Features, comma separated (or blank): ";
        std::getline(std::cin, line);
        std::stringstream ss(line);
        std::string feature;
        while (std::getline(ss, feature, ',')) {
            if (!feature.empty() && feature[0] == ' ') feature.erase(0, 1);
            if (!feature.empty()) brief.features.push_back(feature);
        }
        briefEngine->createBrief(brief);
        architect.confirmBrief(briefEngine->getBrief());
        logger.log("Brief created for " + brief.businessName);
    }

    void buildSite() {
        if (!briefEngine->hasCurrentBrief()) { std::cout << "Describe a site first (option 1).\n"; return; }
        auto report = siteBuilder->buildSite(briefEngine->getBrief());
        architect.presentSite(report);
        if (report.ok) {
            memory.record("site", briefEngine->getBrief(), report.files);
            logger.log("Site built for " + briefEngine->getBrief().businessName);
        }
    }

    void buildProgram() {
        if (!briefEngine->hasCurrentBrief()) { std::cout << "Describe a site first (option 1).\n"; return; }
        auto report = programBuilder->buildProgram(briefEngine->getBrief());
        architect.presentProgram(report);
        if (report.ok) {
            memory.record("program", briefEngine->getBrief(), report.files);
            logger.log("Program built for " + briefEngine->getBrief().businessName);
        }
    }

    void talk() {
        std::cout << "Say something to " << architect.getName() << " (blank to stop):\n";
        std::string line;
        while (true) {
            std::cout << "You: ";
            std::getline(std::cin, line);
            if (line.empty()) break;
            architect.respond(line);
        }
    }

    void showBrief() const { briefEngine->showBrief(); }
    void listProjects() const { memory.listProjects(); }
};

// ======= MAIN =======
void showMenu() {
    std::cout << "\n===== WEBSITE ENGINEERING SITE =====\n"
              << "1. Describe a site (new customer brief)\n"
              << "2. Show the current brief\n"
              << "3. Build the site from the brief\n"
              << "4. Build a program from the brief\n"
              << "5. List built projects\n"
              << "6. Talk to Architect\n"
              << "7. Run demo (Rosewood Bakery)\n"
              << "0. Exit\n"
              << "Choice: ";
}

int main(int argc, char* argv[]) {
    CentralNode centralNode;
    centralNode.initialize();
    // --demo runs the Rosewood Bakery demo with no keyboard input, so
    // automated tests never depend on piping keystrokes into the menu.
    if (argc > 1 && std::string(argv[1]) == "--demo") {
        centralNode.runDemo();
        centralNode.shutdown();
        return 0;
    }
    int choice = -1;
    while (choice != 0) {
        showMenu();
        if (!(std::cin >> choice)) break;
        std::string discard;
        std::getline(std::cin, discard);
        switch (choice) {
            case 1: centralNode.describeSite(); break;
            case 2: centralNode.showBrief(); break;
            case 3: centralNode.buildSite(); break;
            case 4: centralNode.buildProgram(); break;
            case 5: centralNode.listProjects(); break;
            case 6: centralNode.talk(); break;
            case 7: centralNode.runDemo(); break;
            case 0: std::cout << "Goodbye.\n"; break;
            default: std::cout << "Unknown choice.\n"; break;
        }
    }
    centralNode.shutdown();
    return 0;
}

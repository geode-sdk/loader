#include <interface/CLIManager.hpp>

USE_LILAC_NAMESPACE();

CLIManager* CLIManager::get() {
    static auto g_cli = new CLIManager;
    return g_cli;
}

void CLIManager::addCommand(CLICommand cmd) {
    this->m_cmds.push_back(cmd);
}

void CLIManager::execute(std::vector<std::string> incArgs) {
    std::string last_was_flag = "";
    std::unordered_map<std::string, std::string> flags;
    std::vector<std::string> args;
    for (auto const& arg : incArgs) {
        if (arg.c_str()[0] == '-') {
            flags[arg] = "";
            last_was_flag = arg;
        } else if (last_was_flag.size()) {
            flags[last_was_flag] = arg;
            last_was_flag = "";
        } else {
            args.push_back(arg);
        }
    }
    CLIArgs cli = { args, flags };
    for (auto const& cmd : m_cmds) {
        cmd(cli);
    }
}

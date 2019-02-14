#include "cacos/lang/linker.h"

namespace cacos::lang {

Linker::Linker(const cpptoml::table& table, const fs::path& binaryDir)
    : Translator(table)
    , binaryDir_(binaryDir) {
}

/// TODO: remove copypaste from cacos::lang::Compiler::task
fs::path Linker::link(const std::vector<fs::path>& objs, const opts::CompilerOpts& options) const {
    InlineVariables vars;

    fs::path binary = binaryDir_ / (objs[0].filename().string() + "_linked");

    vars.set("arch", util::str(opts::serialize(options.archBits)));
    vars.set("binary", binary.string());
    vars.set("objs", "compiled");

    executable::Flags flags = common_;
    switch (options.buildType) {
    case opts::BuildType::debug:
        flags.append(debug_);
        break;
    case opts::BuildType::release:
        flags.append(release_);
        break;
    default:
        throw std::runtime_error("Unknown build type");
    }
    auto args = flags.build(vars);

    /* add all sources to args */
    ptrdiff_t it = std::find(args.begin(), args.end(), "compiled") - args.begin();
    if (it != static_cast<ptrdiff_t>(args.size())) {
        args.erase(args.begin() + it);
        for (auto rit = objs.rbegin(); rit != objs.rend(); ++rit) {
            args.insert(args.begin() + it, rit->string());
        }
    }

    std::future<std::string> stdOut;
    std::future<std::string> stdErr;

    boost::asio::io_context ctx;
    bp::child child =
        exe_.run(vars, args, bp::std_out > std::ref(stdOut), bp::std_err > std::ref(stdErr), ctx);

    ctx.run_for(std::chrono::seconds(5));

    if (child.running()) {
        child.terminate();
        throw std::runtime_error("Linkage time out");
    }

    if (child.exit_code() != 0) {
        throw std::runtime_error(util::string::join(
            "Cannot link object files: ",
            ":\n\nLinker stdout:\n",
            stdOut.get(),
            "\n\nLinker stderr:\n",
            stdErr.get()));
    }

    return binary;
}

} // namespace cacos::lang

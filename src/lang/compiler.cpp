#include "cacos/lang/compiler.h"

#include "cacos/config/config.h"

#include "cacos/util/inline_variables.h"

#include "cacos/util/logger.h"

namespace cacos::lang {

Compiler::Compiler(const cpptoml::table& t, const fs::path& binaryDir)
    : Translator(t)
    , binaryDir_(binaryDir) {
}

fs::path Compiler::compile(const fs::path& source, const opts::CompilerOpts& opts) const {
    process::Limits limits;
    limits.cpu = limits.real = seconds(3);
    std::future<std::string> stdOut;
    std::future<std::string> stdErr;
    auto&& [binary, task] = this->task(source, opts, stdOut, stdErr);

    executable::ExecPool pool(limits);
    pool.push(std::move(task));
    pool.run();
    return binary;
}

std::pair<fs::path, executable::ExecTaskPtr> Compiler::task(
    const fs::path& source,
    const opts::CompilerOpts& options,
    std::future<std::string>& stdOut,
    std::future<std::string>& stdErr) const {
    InlineVariables vars;

    fs::path binary = binaryDir_ / (source.filename().replace_extension("o"));

    vars.set("source", source.string());
    vars.set("binary", binary.string());
    vars.set("object", binary.string());
    vars.set("arch", util::str(opts::serialize(options.archBits)));

    executable::Flags flags = common_;
    flags.append(debug_);
    auto args = flags.build(vars);

    auto callback = [&, source](process::Result res, std::optional<process::Info>&&) {
        if (res.status == process::status::IL || res.status == process::status::TL) {
            throw std::runtime_error("Compilation time out");
        }
        if (res.returnCode != 0) {
            throw std::runtime_error(util::join(
                "Cannot compile ",
                source.string(),
                ":\n\nCompiler stdout:\n",
                stdOut.get(),
                "\n\nCompiler stderr:\n",
                stdErr.get()));
        }
    };

    auto result = executable::makeTask(
        exe_,
        executable::ExecTaskContext{std::move(args), boost::this_process::environment(), std::move(callback)},
        bp::null,
        std::ref(stdOut),
        std::ref(stdErr));

    return {binary, std::move(result)};
}

} // namespace cacos::lang

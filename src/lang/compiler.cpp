#include "cacos/lang/compiler.h"

#include "cacos/config/config.h"

#include "cacos/util/inline_variables.h"

#include "cacos/util/logger.h"

namespace cacos::lang {

Compiler::Compiler(const cpptoml::table& t, const fs::path& binaryDir)
    : Translator(t)
    , binaryDir_(binaryDir)
{}

executable::Executable Compiler::process(const fs::path& source) const {
    InlineVariables vars;

    fs::path binary = binaryDir_ / source.filename();

    vars.set("source", source);
    vars.set("binary", binary);

    executable::Flags flags = common_;
    flags.append(debug_);
    auto args = flags.build(vars);

    std::future<std::string> stdOut;
    std::future<std::string> stdErr;

    boost::asio::io_context ctx;
    bp::child child = exe_.run(
        vars,
        args,
        bp::std_out > std::ref(stdOut),
        bp::std_err > std::ref(stdErr),
        ctx
    );

    // TODO: fixme
    ctx.run_for(std::chrono::seconds(3));

    if (child.running()) {
        child.terminate();
        throw std::runtime_error("Compilation time out");
    }

    if (child.exit_code() != 0) {
        throw std::runtime_error(util::join(
            "Cannot compile ",
            source.string(),
            ":\n\nCompiler stdout:\n",
            stdOut.get(),
            "\n\nCompiler stderr:\n",
            stdErr.get()));
    }

    if (!stdOut.get().empty()) {
        Logger::warning() << "Compiler stdout:\n" << stdOut.get();
    }

    if (!stdErr.get().empty()) {
        Logger::warning() << "Compiler stderr:\n" << stdErr.get();
    }

    return executable::Executable(binary);
}

} // namespace cacos::lang

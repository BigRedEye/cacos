#include "cacos/lang/compiler.h"

#include "cacos/config.h"

#include "cacos/util/inline_variables.h"

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

    std::future<std::string> stdOut;
    std::future<std::string> stdErr;

    executable::Flags flags = common_;
    flags.append(debug_);
    auto args = flags.build(vars);

    boost::asio::io_context ctx;
    bp::child child = exe_.run(
        vars,
        args,
        bp::std_out > std::ref(stdOut),
        bp::std_err > std::ref(stdErr),
        ctx
    );

    // TODO: fixme
    ctx.run_for(std::chrono::seconds(2));

    if (child.running()) {
        child.terminate();
        throw std::runtime_error("Compilation time out");
    }

    return executable::Executable(binary);
}

} // namespace cacos::lang

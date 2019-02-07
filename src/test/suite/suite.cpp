#include "cacos/test/suite/suite.h"

namespace cacos::test {

Suite::Suite(config::Config& cfg)
    : workspace_(cfg.dir(config::DirType::test))
{}

}

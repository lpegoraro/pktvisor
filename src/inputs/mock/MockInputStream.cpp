/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "MockInputStream.h"

namespace visor::input::mock {

MockInputStream::MockInputStream(const std::string &name)
    : visor::InputStream(name)
{
    _logger = spdlog::get("visor");
    assert(_logger);
    _logger->info("mock input created");
}
MockInputStream::~MockInputStream()
{
    _logger->info("mock input destroyed");
}

void MockInputStream::start()
{

    if (_running) {
        return;
    }

    _logger->info("mock input start()");

    static timer timer_thread{500ms};
    std::srand(std::time(nullptr));
    _mock_work = timer_thread.set_interval(1s, [this] {
        _logger->info("mock input sends random int signal");
        random_int_signal(std::rand());
    });

    _running = true;
}

void MockInputStream::stop()
{
    if (!_running) {
        return;
    }

    _logger->info("mock input stop()");

    _mock_work->cancel();

    _running = false;
}

void MockInputStream::info_json(json &j) const
{
    common_info_json(j);
}

}

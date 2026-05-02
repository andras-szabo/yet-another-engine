#include <iostream>
#include <string>
#include "LoggerMacros.h"

// Import the engine-core module.
// Once engine-core exports real symbols, they will be available here.
import EngineCore;
import EngineError;
import Logger;

Expected<int> ShouldBeEven(int number)
{
    if (number % 2 == 0)
    {
        return number;
    }

    return std::unexpected<EngineError>({ErrorType::LogicError, "Bloargh"});
}

int main()
{
    LOG_INFO("This is an info");
    auto bar = ShouldBeEven(42);
    auto baz = ShouldBeEven(3);

    if (bar.has_value())
    {
        LOG_INFO("OK, bar checks out!, and its value is {}", bar.value());
    }

    if (!baz.has_value())
    {
        LOG_ERROR("Oho. {}", baz.error().message);
    }

    return 0;
}

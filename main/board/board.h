#pragma once

#include "system/interrupt_manager.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

class Board final
{
  public:
    Board(std::shared_ptr<InterruptManager> interrupt_manager);
    ~Board();

    enum MessageType { HELLO, SAY_COMMAND, TIMEOUT, COMMAND_ACCEPTED };

    bool show_message(MessageType type, const char *message = nullptr);
    bool hide_message();

  private:
    struct Data;
    std::unique_ptr<Data> m_data;
};
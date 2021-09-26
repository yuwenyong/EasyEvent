//
// Created by yuwenyong on 2020/12/4.
//

#ifndef EASYEVENT_EVENT_INTERRUPTER_H
#define EASYEVENT_EVENT_INTERRUPTER_H

#include "EasyEvent/Event/EventBase.h"


namespace EasyEvent {

    class EASY_EVENT_API Interrupter: public Selectable {
    public:
        Interrupter(const Interrupter&) = delete;
        Interrupter& operator=(const Interrupter&) = delete;

        Interrupter() {
            openSockets();
        };

        virtual ~Interrupter() noexcept ;

        void handleEvents(IOEvents events) override;

        SocketType getFD() const override;

        void closeFD() override;

        void interrupt();
    protected:
        void openSockets();

        bool reset();

        SocketType _reader{InvalidSocket};
        SocketType _writer{InvalidSocket};
    };

}

#endif //EASYEVENT_EVENT_INTERRUPTER_H

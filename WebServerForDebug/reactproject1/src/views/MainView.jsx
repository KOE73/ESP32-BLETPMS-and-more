import React from 'react';
import Button from '@UI/Button';
import TPMSView from '@/components/TPMS';
import MessageList from '@/components/MessageList';
import MessageSender from '@/components/MessageSender';
import CPUTasks from '@/components/CPUTasks';

import { useWebSocketContext } from '../context/WebSocketContext';

function MainView() {
    const { connectionStatus } = useWebSocketContext();

    return (
        <x>
            <h1 className="text-center">Hello ESP32 SPA</h1>
            <div className="d-flex flex-wrap p-2">
                <div className="flex-fill">
                    <Button label="Press me" onClick={() => console.log('AAAA')} />
                </div>
                <div className="flex-fill">
                    <MessageList />
                </div>
                <div className="flex-fill">
                    <MessageSender />
                </div>

                <div class="w-100"> <hr /></div> 
                <div className="flex-fill">
                    <TPMSView id="1" />
                </div>
                <div className="flex-fill">
                    <TPMSView id="2" />
                </div>
                <div className="flex-fill">
                    <TPMSView id="3" />
                </div>
                <div className="flex-fill">
                    <TPMSView id="4" />
                </div>

                <div class="w-100"> <hr /></div> 

                <div className="flex-fill">
                    <CPUTasks />
                </div>
            </div>
        </x>
    );
}

export default MainView;
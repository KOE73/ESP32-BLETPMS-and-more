import React, { useState } from 'react';
//import useWebSocket from 'react-use-websocket';
import Button from './components/UI/Button';
import TPMSView from './components/TPMS';
import MessageList from './components/MessageList';
import MessageSender from './components/MessageSender';
import CPUTasks from './components/CPUTasks';
import MainView from '@views/MainView';
import { useWebSocketContext } from './context/WebSocketContext';

import './App.css';


function App() {
    const [currentView, setCurrentView] = useState('home');
    const [message, setMessage] = useState('');

    //// WebSocket хук (замени URL на твой сервер, например, ws://192.168.1.100/ws)
    //const {
    //    sendJsonMessage,
    //    lastJsonMessage,
    //    readyState,
    //} = useWebSocket('ws://127.0.0.1:5238/ws', {
    //    onOpen: () => console.log('WebSocket open'),
    //    onError: (error) => console.error('Ошибка WebSocket:', error),
    //    shouldReconnect: () => true, // Автоматическое переподключение
    //});

    const { sendJsonMessage,  connectionStatus } = useWebSocketContext();

 
    // Отправка сообщения
    const handleSendMessage = () => {
        if (message.trim()) {
            sendJsonMessage({ action: 'message', payload: message });
            setMessage('');
        }
    };

    // Компоненты "страниц"
    const views = {
        home: () => (<MainView/>),
        control: () => (
            <div>
                <h1 className="text-center">Control</h1>
                <button
                    className="btn btn-primary mb-2"
                    onClick={() => sendJsonMessage({ action: 'toggle' })}
                >
                    Switch LED
                </button>
                {/*<p>Status LED: <span>{lastJsonMessage?.ledStatus ? 'ON' : 'OFF'}</span></p>*/}
                {/*<p>Device: <span>{lastJsonMessage?.sensorValue || 'N/A'}</span></p>*/}
            </div>
        ),
        settings: () => (
            <div>
                <h1 className="text-center">Setup</h1>
                <input
                    type="text"
                    value={message}
                    onChange={(e) => setMessage(e.target.value)}
                    placeholder="Message for ESP32"
                    className="form-control mb-2"
                />
                <button className="btn btn-primary" onClick={handleSendMessage}>
                    Post
                </button>
                {/*<p>Answer: {lastJsonMessage?.payload || 'No data'}</p>*/}
            </div>
        ),
    };

    return (
        <main class="d-flex flex-nowrap">
            <div class="d-flex flex-column flex-shrink-0 p-3 bg-body-tertiary container1">
                <a href="/" class="d-flex align-items-center mb-3 mb-md-0 me-md-auto link-body-emphasis text-decoration-none">
                    <svg class="bi pe-none me-2" width="40" height="32"><use xlink: href="#bootstrap"></use></svg>
                    <span class="fs-4">TPMS+</span>
                </a>
                <hr />
                <ul class="nav nav-pills flex-column mb-auto">
                    <li class="nav-item">
                        <button
                            className={`nav-link ${currentView === 'home' ? 'active' : ''}`}
                            onClick={() => setCurrentView('home')}>
                            <svg class="bi pe-none me-2" width="16" height="16"><use xlink: href="#home"></use></svg>
                            Home
                        </button>
                    </li>
                    <li>
                        <button href="#" class="nav-link link-body-emphasis"
                            className={`nav-link ${currentView === 'control' ? 'active' : ''}`}
                            onClick={() => setCurrentView('control')}
                        >
                            <svg class="bi pe-none me-2" width="16" height="16"><use xlink: href="#speedometer2"></use></svg>
                            Dashboard
                        </button>
                    </li>
                    <li>
                        <button class="nav-link link-body-emphasis"
                            className={`nav-link ${currentView === 'settings' ? 'active' : ''}`}
                            onClick={() => setCurrentView('settings')}
                        >
                            <svg class="bi pe-none me-2" width="16" height="16"><use xlink: href="#table"></use></svg>
                            Setup
                        </button>
                    </li>
                </ul>
                <hr />
                Status: {connectionStatus}

                {/*<nav className="nav nav-pills justify-content-center mb-4">*/}
                {/*    <button*/}
                {/*        className={`nav-link ${currentView === 'home' ? 'active' : ''}`}*/}
                {/*        onClick={() => setCurrentView('home')}*/}
                {/*    >*/}
                {/*        Main*/}
                {/*    </button>*/}
                {/*    <button*/}
                {/*        className={`nav-link ${currentView === 'control' ? 'active' : ''}`}*/}
                {/*        onClick={() => setCurrentView('control')}*/}
                {/*    >*/}
                {/*        Control*/}
                {/*    </button>*/}
                {/*    <button*/}
                {/*        className={`nav-link ${currentView === 'settings' ? 'active' : ''}`}*/}
                {/*        onClick={() => setCurrentView('settings')}*/}
                {/*    >*/}
                {/*        Setup*/}
                {/*    </button>*/}
                {/*</nav>*/}
            </div>
            <div className="card p-3 fade-in">{views[currentView]()}</div>

        </main >
    );
}

export default App;
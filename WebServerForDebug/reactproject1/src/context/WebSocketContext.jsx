import React, { createContext, useContext } from 'react';
import useWebSocket from 'react-use-websocket';

const WebSocketContext = createContext();

// Маппинг статусов подключения
const connectionStatusMap = {
    0: 'Connecting...', // WebSocket.CONNECTING
    1: 'Opened',        // WebSocket.OPEN
    2: 'Closing',       // WebSocket.CLOSING
    3: 'Closed',        // WebSocket.CLOSED
};

export function WebSocketProvider({ url, children }) {
    const websocket = useWebSocket(url, {
        onOpen: () => console.log('WebSocket conected'),
        onClose: () => console.log('WebSocket closed'),
        onError: () => console.error('WebSocket error'),
        shouldReconnect: () => true, 
    });

    // Вычисляем connectionStatus на основе readyState из websocket
    const connectionStatus = connectionStatusMap[websocket.readyState];

    // Добавляем connectionStatus в значение контекста
    const contextValue = {
        ...websocket,
        connectionStatus,
    };
    return (
        <WebSocketContext.Provider value={contextValue}>
            {children}
        </WebSocketContext.Provider>
    );
}


export function useWebSocketContext() {
    return useContext(WebSocketContext);
}
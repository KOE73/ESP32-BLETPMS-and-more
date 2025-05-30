import React, { useEffect, useState } from 'react';
import { useWebSocketContext } from '../context/WebSocketContext';

function MessageList() {
    const [messages, setMessages] = useState([]);
    const { lastMessage, readyState } = useWebSocketContext();

    // ќбновл€ем список сообщений при получении нового
    useEffect(() => {
        if (lastMessage !== null) {
            setMessages((prev) => [...prev, lastMessage.data]);
        }
    }, [lastMessage]);

    return (
        <div className="border p-3 mb-3" style={{ maxHeight: '300px', overflowY: 'auto' }}>
            <h3>Messages</h3>
            <p>State: {readyState === 1 ? 'Conected' : 'Disconected'}</p>
            {messages.length === 0 ? (
                <p>No message</p>
            ) : (
                <ul className="list-group">
                    {messages.map((msg, index) => (
                        <li key={index} className="list-group-item">
                            {msg}
                        </li>
                    ))}
                </ul>
            )}
        </div>
    );
}

export default MessageList;
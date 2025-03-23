import React, { useState } from 'react';
import { useWebSocketContext } from '../context/WebSocketContext';

function MessageSender() {
    const [input, setInput] = useState('');
    const { sendMessage, readyState } = useWebSocketContext();

    const handleSubmit = (e) => {
        e.preventDefault();
        if (input.trim() && readyState === 1) {
            sendMessage(input);
            setInput('');
        }
    };

    return (
        <form onSubmit={handleSubmit} className="input-group">
            <input
                type="text"
                className="form-control"
                value={input}
                onChange={(e) => setInput(e.target.value)}
                placeholder="Input message"
                disabled={readyState !== 1}
            />
            <button type="submit" className="btn btn-primary" disabled={readyState !== 1}>
                Post
            </button>
        </form>
    );
}

export default MessageSender;
import React, { useEffect, useState } from 'react';
import { useWebSocketContext } from '../context/WebSocketContext';

function TPMSView({ id }) {
    const [status, setStatus] = useState('');
    const { lastMessage } = useWebSocketContext();

    useEffect(() => {
        if (lastMessage !== null) {
            const message = JSON.parse(lastMessage.data);
            if (message.msgType === 'tpms' && message.id === id) {
                setStatus(message);
            }
        }
    }, [lastMessage, id]);

    //function TPMSView({ label, onClick }) {
    return (
        <div className="card " style={{ width: '6rem' }}>
            <div className="card-header">
                {id}
            </div>
            <div className="card-body">
                <h2>{status.pressure_Bar?.toFixed(1) || 'N/A'}</h2>
                <h3>{status.temperatureC?.toFixed(1) || 'N/A'}℃</h3>
            </div>
        </div>

    );
}


export default TPMSView;
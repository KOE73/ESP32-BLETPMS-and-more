import React, { useEffect, useState } from 'react';
import { useWebSocketContext } from '../context/WebSocketContext';

function formatNumberWithThinSpaces(number) {
    // \u00A0 \u2009
    return number;//.toLocaleString('en-US', { useGrouping: true }).replace(/,/g, '\u00A0');
}
function CPUTasks() {
    const [diagnosticData, setDiagnosticData] = useState(null);
    const { lastMessage, readyState } = useWebSocketContext();

    useEffect(() => {
        if (lastMessage !== null) {
            const message = JSON.parse(lastMessage.data);
            if (message.msgType === 'diagnostic') {
                setDiagnosticData(message);
            }
        }
    }, [lastMessage]);

    // Проверка готовности подключения и наличия задач
    if (readyState !== 1) {
        return (
            <div className="container mt-5">
                <p>Not connected WebSocket...</p>
            </div>
        );
    }

    if (!diagnosticData || !diagnosticData.tasks || diagnosticData.tasks.length === 0) {
        return (
            <div className="container mt-5">
                <p>Tasks not found</p>
            </div>
        );
    }

    //const sortedTasks = [diagnosticData].sort((a, b) => b.rp - a.rp);

    //console.log('diagnosticData:', diagnosticData);
    //console.log('diagnosticData.tasks:', diagnosticData.tasks);

    //function TPMSView({ label, onClick }) {
    return (
        <div class="card">
            <div className="card-header">
                CPU
            </div>
            <div class="card-body">
                <div className="diagnostic-grid-container">
                    {/* Заголовки */}
                    <div className="grid-header">Name</div>
                    <div className="grid-header">Task#</div>
                    <div className="grid-header">S</div>
                    <div className="grid-header">P</div>
                    <div className="grid-header">Stack<br />Free</div>
                    <div className="grid-header">%</div>
                    {/* <div className="grid-header">Runtime</div> */}

                    {/* Данные задач */}
                    {[...diagnosticData.tasks]
                        .sort((a, b) => b.rp - a.rp)
                        .map((task, index) => (
                            <React.Fragment key={index}>
                                <div className="grid-item">{task.name}</div>
                                <div className="grid-item r">{task.tn}</div>
                                <div className="grid-item">{task.s}</div>
                                <div className="grid-item r">{task.p}</div>
                                <div className="grid-item r">{task.sf}</div>
                                <div className="grid-item r">{task.rp}%</div>
                            </React.Fragment>
                        ))}

                    {/* Итоговые данные */}
                    <div className="grid-item total" style={{ gridColumn: 'span 6', textAlign: 'left' }}>
                        Total Tasks: {diagnosticData.total_tasks}, Free Heap:{' '}
                        {formatNumberWithThinSpaces(diagnosticData.free_heap)} bytes
                    </div>

                </div></div>
        </div>
    );
}


export default CPUTasks;
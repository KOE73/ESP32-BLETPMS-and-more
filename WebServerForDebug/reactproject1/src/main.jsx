import { StrictMode } from 'react'
import { createRoot } from 'react-dom/client'
import 'bootstrap/dist/css/bootstrap.min.css'; // Импорт стилей
//import 'bootstrap'; // Импорт JS (включает Popper.js)
import { WebSocketProvider } from './context/WebSocketContext';

import App from './App.jsx'
import './index.css'

createRoot(document.getElementById('root')).render(
    <WebSocketProvider url="ws://127.0.0.1:5238/ws">
        <StrictMode>
            <App />
        </StrictMode>
    </WebSocketProvider>
    ,
)

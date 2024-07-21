import { proxy } from "valtio";

export enum WsMessageType {
    Ping,
    Pong,
    Error,
    RequestJoinRoom,
    AllowJoinRoom,
    DenyJoinRoom,
    RoomReady,
}

export enum WsConnectionState {
    Connecting,
    Connected,
    Disconnected,
};

export enum PeerConnectionState {
    Waiting,
    Connecting,
    Connected,
    Disconnected,
    ReceivdRequest,
    Denied
}

type JoinRoomRequest = {
    requestId: string,
    roomId: string,
    userId: string,
    username: string,
}

type State = {
    wsConnectionState: WsConnectionState,
    peerConnectionState: PeerConnectionState,
    joinRoomRequest: JoinRoomRequest | null,
}

class Connection {

    private _connection: WebSocket | null = null;
    private _joinRoomRequestId: string | null = null;
    private _roomId: string | null = null;
    private _isHost = false;


    public state = proxy<State>({
        wsConnectionState: WsConnectionState.Disconnected,
        peerConnectionState: PeerConnectionState.Waiting,
        joinRoomRequest: null,
    });

    public async connect(url: string) {
        if (this._connection) {
            this._connection.close();
        }
        this.state.wsConnectionState = WsConnectionState.Connecting;
        return new Promise<void>((resolve, reject) => {
            this._connection = new WebSocket(url);
            this._connection.onmessage = (event) => {
                this.onMessage(JSON.parse(event.data));
            };
            this._connection.onclose = () => {
                console.log("Disconnected from server");
                this.state.wsConnectionState = WsConnectionState.Disconnected;
                this._connection = null;
            }
            this._connection.onerror = () => {
                this._connection = null;
            }
            this._connection.onopen = () => {
                console.log("Connected to server");
                this.state.wsConnectionState = WsConnectionState.Connected;
                resolve();
            }
            // timeout after 5 seconds
            setTimeout(() => {
                if (this._connection && this._connection.readyState !== WebSocket.OPEN) {
                    reject("Failed to connect to server");
                    this._connection.close();
                    this._connection = null;
                }
            }, 5000);
        });
    }

    public send(message: any) {
        if (this._connection) {
            this._connection.send(JSON.stringify(message));
        }
    }

    public setJoinRoomRequestId(requestId: string) {
        this._joinRoomRequestId = requestId;
    }

    public setRoomId(roomId: string) {
        this._roomId = roomId;
    }

    public setIsHost(isHost: boolean) {
        this._isHost = isHost;
    }

    public close() {
        if (this._connection) {
            this._connection.close();
        }
    }

    private onMessage(message: any) {
        console.log("Received message", message);

        switch (message.type) {
            case WsMessageType.RequestJoinRoom:
                if (this._isHost) {
                    this.state.peerConnectionState = PeerConnectionState.ReceivdRequest;
                    this.state.joinRoomRequest = message.payload
                }
                break;

            case WsMessageType.AllowJoinRoom:
                    console.log("Joined room");
                break;

            case WsMessageType.DenyJoinRoom:
                console.log("Denied to join room", this._joinRoomRequestId, message.payload.requestId);
                if (message.payload.requestId === this._joinRoomRequestId) {
                    console.log("Failed to join room");
                    this.state.peerConnectionState = PeerConnectionState.Denied;
                }
                break;



            case WsMessageType.RoomReady:
                if (message.payload.roomId === this._roomId) {
                    console.log("Room is ready");
                    this.state.peerConnectionState = PeerConnectionState.Connecting;
                    // TODO: start peer connection
                }
                break;

            default:
                break;
        }
    }
}

export const connection = new Connection();

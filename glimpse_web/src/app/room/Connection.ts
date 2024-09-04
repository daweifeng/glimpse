import { exchangeICE, exchangeSDP } from "@/utils/api";
import { proxy } from "valtio";

export enum WsMessageType {
    Ping,
    Pong,
    Error,
    RequestJoinRoom,
    AllowJoinRoom,
    DenyJoinRoom,
    RoomReady,
    RoomEnd,
    SDP,
    ICE
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
    private _peerConnection: RTCPeerConnection | null = null;
    private _pendingICEs: string[] = [];


    public state = proxy<State>({
        wsConnectionState: WsConnectionState.Disconnected,
        peerConnectionState: PeerConnectionState.Waiting,
        joinRoomRequest: null,
    });

    public get roomId() {
        const urlParams = new URLSearchParams(window.location.search);
        return urlParams.get('id');
    }

    public get isHost(): boolean {
        const urlParams = new URLSearchParams(window.location.search);

        return urlParams.get('is_host') === "true";
    }

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

    public close() {
        if (this._connection) {
            this._connection.close();
        }
    }

    private async onMessage(message: any) {
        console.log("Received message", message);

        switch (message.type) {
            case WsMessageType.RequestJoinRoom:
                if (this.isHost) {
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
                if (message.payload.roomId === this.roomId) {
                    console.log("Room is ready");
                    this.state.peerConnectionState = PeerConnectionState.Connecting;
                    console.log(this.isHost)
                    if (this.isHost) {
                        this.createPeerConnection();
                        this.setUpVideo()
                    }
                }
                break;

            case WsMessageType.ICE:
                if (!this._peerConnection || !this._peerConnection.remoteDescription) {
                    this._pendingICEs.push(message.payload);
                } else {
                    this._peerConnection.addIceCandidate(JSON.parse(message.payload));
                }
                break;

            case WsMessageType.SDP:
                if (!this._peerConnection) {
                    this.createPeerConnection();
                    (this._peerConnection as unknown as RTCPeerConnection).setRemoteDescription(JSON.parse(message.payload));
                    if (this._pendingICEs.length !== 0) {
                        this._pendingICEs.forEach((ice) => {
                            (this._peerConnection as unknown as RTCPeerConnection).addIceCandidate(JSON.parse(ice));
                        })
                        this._pendingICEs = [];
                    }
                    await this.setUpVideo();
                    const answer = await (this._peerConnection as unknown as RTCPeerConnection).createAnswer();
                    const userId = window.localStorage.getItem("userId");
                    if (!userId) {
                        console.error("Missing userId");
                        return;
                    }
                    if (!this.roomId) {
                        console.error("Missing roomId");
                        return;
                    }
                    exchangeSDP(this.roomId, userId, JSON.stringify(answer));
                    (this._peerConnection as unknown as RTCPeerConnection).setLocalDescription(answer);
                } else {
                    this._peerConnection.setRemoteDescription(JSON.parse(message.payload));
                    if (this._pendingICEs.length !== 0) {
                        this._pendingICEs.forEach((ice) => {
                            (this._peerConnection as unknown as RTCPeerConnection).addIceCandidate(JSON.parse(ice));
                        })
                        this._pendingICEs = [];
                    }
                }
                break;
            case WsMessageType.RoomEnd:
                if (message.payload.roomId === this.roomId) {
                    console.log("Room has ended");
                    this.state.peerConnectionState = PeerConnectionState.Disconnected;
                    this._peerConnection?.close();
                    this._peerConnection = null;
                }
                break

            default:
                break;
        }
    }

    createPeerConnection() {
        this._peerConnection = new RTCPeerConnection({
            iceServers: [{ urls: "stun:stun.l.google.com:19302" }],
        });
        this._peerConnection.onicecandidate = (event) => {
            if (event.candidate) {
                const userId = window.localStorage.getItem("userId");
                if (!userId) {
                    console.error("Missing userId");
                    return;
                }
                if (!this.roomId) {
                    console.error("Missing roomId");
                    return;
                }
                exchangeICE(this.roomId, userId, JSON.stringify(event.candidate.toJSON()));
            }
        }
        this._peerConnection.onconnectionstatechange = () => {
            console.log("Peer Connection State: ", this._peerConnection?.connectionState);
            if (this._peerConnection?.connectionState === "connected") {
                this.state.peerConnectionState = PeerConnectionState.Connected;
            }
        }
        this._peerConnection.onnegotiationneeded = async (event) => {
            if (this._peerConnection?.remoteDescription) {
                return;
            }
            const userId = window.localStorage.getItem("userId");
            const offer = await this._peerConnection?.createOffer();
            if (!userId) {
                console.error("Missing userId");
                return;
            }
            if (!this.roomId) {
                console.error("Missing roomId");
                return;
            }
            exchangeSDP(this.roomId, userId, JSON.stringify(offer));
            this._peerConnection?.setLocalDescription(offer);
        }

        this._peerConnection.ontrack = (event) => {
            const video = document.querySelector<HTMLVideoElement>("#remote-video");
            if (!video) {
                console.error("could not find video element");
                return;
            }
            video.srcObject = event.streams[0];
            video.onloadedmetadata = () => {
                video.play();
            };
        }
    }

    async setUpVideo() {
        let stream = null;

        try {
            stream = await navigator.mediaDevices.getUserMedia({ video: true, audio: true });
            this._peerConnection?.addTrack(stream.getVideoTracks()[0], stream);
            this._peerConnection?.addTrack(stream.getAudioTracks()[0], stream);
            const video = document.querySelector<HTMLVideoElement>("#self-video");
            if (!video) {
                console.error("could not find video element");
                return;
            }
            video.srcObject = stream;
            video.onloadedmetadata = () => {
                video.play();
            };
          /* use the stream */
        } catch (err) {
          /* handle the error */
          console.error((err as Error).message)
        }

    }
}

export const connection = new Connection();

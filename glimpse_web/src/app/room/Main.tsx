"use client";

import { useRouter, useSearchParams } from "next/navigation";
import { useEffect, useState } from "react";
import { connection, PeerConnectionState, WsConnectionState } from "./Connection";
import { approveJoinRoom, denyJoinRoom, joinRoom, serverWsUrl } from "@/utils/api";
import { useSnapshot } from "valtio";
import Video from "./Video";

export default function ConnectionContainer() {
    const snap = useSnapshot(connection.state);
    const router = useRouter();
    const searchParams = useSearchParams();
    const roomId = searchParams.get("id");
    const isHost = searchParams.get("is_host");

    const handleApproveJoinRoom = () => {
        try {
            const userId = window.localStorage.getItem("userId");
            const requestId = connection.state.joinRoomRequest?.requestId;
            if (!userId || !requestId) {
                throw new Error("missing userId or requestId")
            }
            approveJoinRoom(userId,requestId);
        } catch (error) {
            console.error((error as Error).message)
        }
    }

    const handleDenyJoinRoom = () => {
        try {
            const userId = window.localStorage.getItem("userId");
            const requestId = connection.state.joinRoomRequest?.requestId;
            if (!userId || !requestId) {
                throw new Error("missing userId or requestId")
            }
            denyJoinRoom(userId,requestId);
            connection.state.peerConnectionState = PeerConnectionState.Waiting;
        } catch (error) {
            console.error((error as Error).message)
        }
    }


    useEffect(() => {
        const userId = window.localStorage.getItem("userId");
        const username = window.localStorage.getItem("username");
        if (!roomId || !userId || !username) {
            router.push("/");
            return;
        }
        connection.connect(`${serverWsUrl}/ws?&userId=${userId}&username=${username}`)
            .then(async () => {
                const response = await joinRoom(username, userId, roomId);
                connection.setJoinRoomRequestId(response.requestId);
            }).catch((err: Error) => {
                console.error(err);
                router.push("/");
            });

        return () => {
            connection.close();
        }
    }, [roomId, router, isHost]);

    if (snap.peerConnectionState === PeerConnectionState.Waiting) {
        return (
            <div className="text-center">
                <div className="text-3xl font-bold">Waiting for connection... </div>
                <div>
                    Room Id: <input value={roomId ?? ""} disabled size={40}/>
                </div>
                <div>
                    <button className="bg-blue-500 hover:bg-blue-700 text-white py-2 px-4 rounded transition ease-in-out delay-150" onClick={() => {
                        navigator.clipboard.writeText(roomId ?? "")
                    }}>copy</button>
                </div>
            </div>
        )
    }

    if (snap.peerConnectionState === PeerConnectionState.Denied) {
        return <div>Call declined. Perhaps try again later?</div>
    }

    if (snap.peerConnectionState === PeerConnectionState.ReceivdRequest) {
        return (
            <div className="flex flex-col text-center">
                <div>{snap.joinRoomRequest?.username} wants to join your room</div>
                <button className="min-w-40 bg-green-500 hover:bg-green-700 text-white font-bold mt-2 py-2 px-4 rounded transition ease-in-out delay-150" onClick={handleApproveJoinRoom}>Allow</button>
                <button className="min-w-40 bg-red-500 hover:bg-red-700 text-white font-bold mt-2 py-2 px-4 rounded transition ease-in-out delay-150" onClick={handleDenyJoinRoom}>Deny</button>
            </div>
        )
    }

    return (
        <div>
            <Video id={"self-video"} name={"self"} />
            <Video id={"remote-video"} name={"remote"} />
        </div>
    );
}

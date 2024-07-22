"use client";

import { useRouter, useSearchParams } from "next/navigation";
import { useEffect, useState } from "react";
import { connection, PeerConnectionState, WsConnectionState } from "./Connection";
import { approveJoinRoom, denyJoinRoom, joinRoom, serverWsUrl } from "@/utils/api";
import { useSnapshot } from "valtio";

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
                connection.setRoomId(roomId);
                connection.setIsHost(isHost === "true");
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
        return <div>Waiting for connection...</div>
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

    if (snap.peerConnectionState === PeerConnectionState.Connecting) {
        return <div>

            Connecting...
            </div>
    }

    if (snap.peerConnectionState === PeerConnectionState.Connected) {
        return (
        <div>
            Connected..
        </div>
        )
    }

    return (
        <div>
                Ugg..
        </div>
    );
}

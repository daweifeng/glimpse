"use client"

import { Suspense } from "react";
import Main from "./Main"

export default function RoomPage() {

    return (
        <main className="flex min-h-screen flex-col items-center justify-center p-24">
            <Suspense>
                <Main />
                <div>
                    <div>Remote</div>
                    <video id="remote-video" muted autoPlay playsInline></video>
                </div>
                <div>
                    <div>Self</div>
                    <video id="self-video" muted autoPlay playsInline></video>
                </div>
            </Suspense>
        </main>
    );
}

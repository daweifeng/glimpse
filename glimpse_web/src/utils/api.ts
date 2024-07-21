
export const serverApiUrl = `${process.env.NEXT_PUBLIC_SERVER_SECURE == "true" ? "https" : "http"}://${process.env.NEXT_PUBLIC_SERVER_HOST}:${process.env.NEXT_PUBLIC_SERVER_PORT}`;
export const serverWsUrl = `${process.env.NEXT_PUBLIC_SERVER_SECURE == "true" ? "wss" : "ws"}://${process.env.NEXT_PUBLIC_SERVER_HOST}:${process.env.NEXT_PUBLIC_SERVER_PORT}`;

export const createRoom = async (username: string, userId: string) => {
    const response = await fetch(`${serverApiUrl}/room`, {
        method: "POST",
        body: JSON.stringify({ username, userId }),
        headers: {
            "Content-Type": "application/json",
        },
    });
    if (!response.ok) {
        throw new Error("Failed to create room");
    }
    return await response.json();
}


export const joinRoom = async (username: string, userId: string, roomId: string) => {
    const response = await fetch(`${serverApiUrl}/room/join`, {
        method: "POST",
        body: JSON.stringify({ username, userId, roomId }),
        headers: {
            "Content-Type": "application/json",
        },
    });
    if (!response.ok) {
        throw new Error("Failed to join room");
    }
    return await response.json();
}


export const approveJoinRoom = async (userId:string, requestId: string) => {
    const response = await fetch(`${serverApiUrl}/room/join/approve`, {
        method: "POST",
        body: JSON.stringify({ userId, requestId }),
        headers: {
            "Content-Type": "application/json",
        },
    });
    if (!response.ok) {
        throw new Error("Failed to allow join room");
    }
    return await response.json();
}

export const denyJoinRoom = async (userId:string, requestId: string) => {
    const response = await fetch(`${serverApiUrl}/room/join/deny`, {
        method: "POST",
        body: JSON.stringify({ userId, requestId }),
        headers: {
            "Content-Type": "application/json",
        },
    });
    if (!response.ok) {
        throw new Error("Failed to deny join room");
    }
    return await response.json();
}

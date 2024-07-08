'use client';
import { createRoom } from "@/utils/api";
import { useFormStatus } from "react-dom";
import { v4 as uuidv4 } from "uuid";

function StartButton() {
    const status = useFormStatus();

    return (
        <button
            className="min-w-40 bg-blue-500 hover:bg-blue-700 disabled:bg-gray-400 disabled:cursor-not-allowed text-white font-bold mt-2 py-2 px-4 rounded transition ease-in-out delay-150 "
            type="submit"
            disabled={status.pending}
        >
            { status.pending ? "Starting..." : "Start"}
        </button>
    )
}

export default function NewPage() {

    const handleSubmit = async (data: FormData) => {
        const username = data.get("username");
        try {
            if (username) {
                const response = await createRoom(username.toString(), uuidv4());
                console.log(response)
            } else {
                // Show error
            }
        } catch (error) {
            console.error(error);
        }
    };


    return (
        <main className="flex min-h-screen flex-col items-center justify-center p-24">
            <form className="flex flex-col justify-around flex-wrap m-auto p-8" action={handleSubmit}>
                <label className="" htmlFor="username">
                    Your name
                </label>
                <input required id="username" name="username" className="border border-slate-300 rounded p-2" type="text" />
                <StartButton />
            </form>
        </main>
    );
}

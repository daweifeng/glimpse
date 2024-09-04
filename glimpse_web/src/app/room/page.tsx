"use client";

import { Suspense } from "react";
import Main from "./Main";

export default function RoomPage() {
  return (
    <main className="flex min-h-screen flex-col items-center justify-center p-24">
      <Suspense>
        <Main />
      </Suspense>
    </main>
  );
}

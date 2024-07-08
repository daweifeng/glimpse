import Image from "next/image";

export default function Home() {
  return (
    <main className="flex min-h-screen flex-col items-center justify-center p-24">
      <div>
        <h1 className="text-center">
          Glimpse
        </h1>
        <p className="text-slate-500 text-center pt-4">
          No sign up, no tracking, just video chat.
        </p>
        <div className="flex flex-col justify-around flex-wrap m-auto p-8">
          <button className="min-w-40 bg-blue-500 hover:bg-blue-700 text-white font-bold mt-2 py-2 px-4 rounded transition ease-in-out delay-150">
            <a href="/new">Create Room</a>
          </button>
          <button className="min-w-40 bg-blue-500 hover:bg-blue-700 text-white font-bold mt-2 py-2 px-4 rounded transition ease-in-out delay-150">
            Join Room
          </button>
        </div>
      </div>
      <footer className="absolute bottom-0 w-full text-center text-slate-800 p-4 text-sm font-light">
        <div className="w-6 m-auto pb-3">
          <a target="_blank" href="https://github.com/daweifeng/glimpse" className="m-auto">
            <Image className="opacity-70 hover:opacity-90 transition ease-in-out delay-150 cursor-pointer" src="/github-mark.svg" alt="Github Logo" width={24} height={24} />
          </a>
        </div>
        <p>
          Built with ❤️ by Dawei
        </p>
      </footer>
    </main>
  );
}

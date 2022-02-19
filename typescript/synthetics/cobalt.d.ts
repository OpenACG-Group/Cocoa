export function renderHostInitialize(application: string): void;
export function renderHostDispose(): void;

export interface RenderClientError extends Error {
    opcode: number;
}

type SlotID = number;
export class RenderClientObject {
    connect(signal: string, slot: Function): SlotID;
    disconnect(id: SlotID): void;
}

export class Display extends RenderClientObject {
    close(): Promise<void>;
}

export function connect(name?: string): Promise<Display>;

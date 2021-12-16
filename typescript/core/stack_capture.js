export function StackCapture() {
    Error.captureStackTrace(this);
}

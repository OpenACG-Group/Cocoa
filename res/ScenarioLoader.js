export function print(str)
{
    Cocoa.core.opCall(Cocoa.core.OP_PRINT, {str: str});
}

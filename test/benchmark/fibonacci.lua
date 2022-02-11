local function fib(x)
    if x == 1 or x == 2 then
        return 1
    end
    return fib(x - 1) + fib(x - 2)
end

local st = os.clock()
local result = fib(40)
local et = os.clock()

print('result = ', result, ', ', et - st)

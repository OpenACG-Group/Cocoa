let builder = ...;

let mainBlock = builder.getUserMainEntrypoint();

mainBlock.emit((block) => {
    let u = block.arg(0);
    let v = block.newFloat2(1.0, 2.0);
    block.createIf(block.createCmpEQ(u, v))
        .createThen((block) => {
                        
                    })
        .createElse((block) => {});
});

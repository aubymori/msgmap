const yaml = require("yaml");
const { argv } = require("node:process");

function printUsage()
{
    console.log(
`USAGE: node msgmap.js [options] <dir>
       node msgmap.js <--help|-h>

Options:
    --help, -h:
        Displays this help message.

    --out-dir, -o:
        Output directory for the translations.
        
    --wide, -w:
        Use wide character (wchar_t) strings for
        the translations. The produced headers will
        only work with the Microsoft Visual C++
        compiler.`);
}

if (argv.length < 3
|| argv[2] == "--help" || argv[2] == "-h")
{
    printUsage();
    return;
}

for (let i = 2; i < argv.length; i++)
{
    console.log(argv[i]);
}
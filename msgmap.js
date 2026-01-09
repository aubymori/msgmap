const yaml = require("yaml");
const { argv } = require("node:process");
const fs = require("fs");

const LOCALE_REGEX = /^([a-z]{1,31})(?:()|_([A-Z]{1,31}))$/;
const PRINTF_FORMAT_REGEX = /^([+\- ]+|)([0-9]+|)(?:\.([0-9]+)|())(hh|h|l|ll|j|z||t|L|)([diouxXfFeEgGaAcsP])$/;
const C_VAR_REGEX = /^([a-zA-Z_][1-9a-zA-Z_]+)$/;

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
        "build" by default.
        
    --wide, -w:
        Use wide character (wchar_t) strings for
        the translations. The produced headers will
        only work with the Microsoft Visual C++
        compiler.
        
    --default-lang, -d:
        Default language to use if the user does
        not prefer any of the provided languages.
        "en_US" by default.`);
}

if (argv.length < 3
|| argv[2] == "--help" || argv[2] == "-h")
{
    printUsage();
    return;
}

let g_wideChars   = false;
let g_outDir      = "build";
let g_inDir       = null;
let g_defaultLang = "en_US";

for (let i = 2; i < argv.length; i++)
{
    if (argv[i] == "-w" || argv[i] == "--wide")
    {
        g_wideChars = true;
    }
    else if (argv[i] == "-o" || argv[i] == "--out-dir")
    {
        if (i == (argv.length - 1))
        {
            console.log(`FATAL: ${argv[i]} provided with no value`);
            return;
        }

        g_outDir = argv[++i];
    }
    else if (argv[i] == "-d" || argv[i] == "--default-language")
    {
        if (i == (argv.length - 1))
        {
            console.log(`FATAL: ${argv[i]} provided with no value`);
            return;
        }

        g_defaultLang = argv[++i];
    }
    else if (i == (argv.length - 1))
    {
        g_inDir = argv[i];
    }
    else
    {
        console.log(`FATAL: Unrecognized option: ${argv[i]}`);
        return;
    }
}

if (!g_inDir)
{
    console.log("FATAL: Translations directory was not provided");
    return;
}

if (!fs.existsSync(g_inDir) || !fs.lstatSync(g_inDir).isDirectory())
{
    console.log(`FATAL: Translations directory '${g_inDir}' does not exist or is not a directory.`);
    return;
}

console.log(fs.readdirSync(g_inDir));
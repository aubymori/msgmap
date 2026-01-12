const yaml         = require("yaml");
const { argv }     = require("node:process");
const fs           = require("fs");

const LOCALE_REGEX            = /^([a-z]{1,31})(?:()|_([A-Z]{1,31}))$/;
const PRINTF_FORMAT_REGEX     = /^([+\- ]+|)([0-9]+|)(?:\.([0-9]+)|())(hh|h|l|ll|j|z||t|L|)([diouxXfFeEgGaAcsP])$/;
const C_VAR_REGEX             = /^([a-zA-Z_][1-9a-zA-Z_]+)$/;

const LogType = {
    INFO:    0,
    WARNING: 1,
    FATAL:   2,
};

function log(type, text)
{
    switch (type)
    {
        case LogType.INFO:
            console.log(`INFO: ${text}`);
            break;
        case LogType.WARNING:
            console.log(`\x1b[93mWARNING: ${text}\x1b[39m`);
            break;
        case LogType.FATAL:
            console.log(`\x1b[91mFATAL: ${text}\x1b[39m`);
            break;
    }
}

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

// Parse arguments
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
            log(LogType.FATAL, `${argv[i]} provided with no value`);
            return;
        }

        g_outDir = argv[++i];
    }
    else if (argv[i] == "-d" || argv[i] == "--default-language")
    {
        if (i == (argv.length - 1))
        {
            log(LogType.FATAL, `${argv[i]} provided with no value`);
            return;
        }

        let lang = argv[++i];
        if (!lang.match(LOCALE_REGEX))
        {
            log(LogType.FATAL, `Invalid value '${lang}' provided for ${argv[i]} option`);
            return;
        }
        g_defaultLang = lang;
    }
    else if (i == (argv.length - 1))
    {
        g_inDir = argv[i];
    }
    else
    {
        log(LogType.FATAL, `Unrecognized option: ${argv[i]}`);
        return;
    }
}

if (!g_inDir)
{
    log(LogType.FATAL, "Translations directory was not provided");
    return;
}

if (!fs.existsSync(g_inDir) || !fs.lstatSync(g_inDir).isDirectory())
{
    log(LogType.FATAL, `Translations directory '${g_inDir}' does not exist or is not a directory.`);
    return;
}

let subfolders = fs.readdirSync(g_inDir);

if (!subfolders.includes(g_defaultLang) || !fs.lstatSync(`${g_inDir}/${g_defaultLang}`).isDirectory())
{
    log(LogType.FATAL, `Default language '${g_defaultLang}' does not have an accompanying directory in the translations directory.`);
    return;
}

let recordList = [];
let defaultRecords = fs.readdirSync(`${g_inDir}/${g_defaultLang}`);
for (const recordName of defaultRecords)
{
    let recordPath = `${g_inDir}/${g_defaultLang}/${recordName}`;
    if (fs.lstatSync(recordPath).isDirectory())
        continue;

    if (recordName.endsWith(".yml") || recordName.endsWith(".yaml"))
        recordList.push(recordName.replace(/\.y(a|)ml$/, ""));
}

let langList = [g_defaultLang];
for (const lang of subfolders)
{
    let langPath = `${g_inDir}/${lang}`;
    if (!fs.lstatSync(langPath).isDirectory())
        continue;

    if (!lang.match(LOCALE_REGEX))
    {
        log(LogType.WARNING, `Skipping subdirectory '${lang}' as it is not a valid locale name.`);
        continue;
    }

    langList.push(lang);

    let langRecords = fs.readdirSync(langPath);
    for (const recordName of recordList)
    {
        if (!langRecords.includes(`${recordName}.yml`)
        && !langRecords.includes(`${recordName}.yaml`))
        {
            log(LogType.FATAL, `Language '${lang}' does not include record '${recordName}'`);
            return;
        }
    }
}
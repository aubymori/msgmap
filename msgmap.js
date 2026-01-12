const yaml         = require("yaml");
const { argv }     = require("node:process");
const fs           = require("fs");

const LOCALE_REGEX            = /^([a-z]{1,31})(?:()|_([A-Z]{1,31}))$/;
const FORMATTED_STRING_REGEX  = /\%([a-zA-Z_][a-zA-Z1-9_]+):(([+\- ]+|)([0-9]+|)(?:\.([0-9]+)|)(hh|h|l|ll|j|z||t|L|)([diouxXfFeEgGaAcsP]))\%/g;
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

let g_recordList = [];
let defaultRecords = fs.readdirSync(`${g_inDir}/${g_defaultLang}`);
for (const recordName of defaultRecords)
{
    let recordPath = `${g_inDir}/${g_defaultLang}/${recordName}`;
    if (fs.lstatSync(recordPath).isDirectory())
        continue;

    if (recordName.endsWith(".yml"))
        g_recordList.push(recordName.replace(/\.yml$/, ""));
}

let g_langList = [g_defaultLang];
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
    
    if (lang == g_defaultLang)
        continue;

    g_langList.push(lang);

    let langRecords = fs.readdirSync(langPath);
    for (const recordName of g_recordList)
    {
        if (!langRecords.includes(`${recordName}.yml`))
        {
            log(LogType.FATAL, `Language '${lang}' does not include record '${recordName}'`);
            return;
        }
    }
}

function parseRecord(lang, record)
{
    let result = { "strings": {}, "formattedStrings": {} };
    
    let path = `${g_inDir}/${lang}/${record}.yml`;
    let text = fs.readFileSync(path).toString();
    let obj;
    try
    {
        obj = yaml.parse(text);
    }
    catch (e)
    {
        log(LogType.FATAL, `Failed to parse YAML object in '${path}': ${e.message}`);
        return null;
    }

    if (Array.isArray(obj))
    {
        log(LogType.FATAL, `Data in '${path}' is an array, it must be an object`);
        return null;
    }

    for (const key in obj)
    {
        if (!key.match(C_VAR_REGEX))
        {
            log(LogType.FATAL, `String name '${key}' in '${path}' is not a valid C variable name.`);
            return null;
        }

        let str = obj[key];
        if (typeof str !== "string")
        {
            log(LogType.FATAL, `Property '${key}' in '${path}' is not a string.`);
            return null;
        }

        let matches = Array.from(str.matchAll(FORMATTED_STRING_REGEX));
        if (matches.length == 0)
        {
            result.strings[key] = str;
        }
        else
        {
            let args = [];
            for (const match of matches)
            {
                let cType = "";
                let [ _, name, format, flags, width, precision, length, type ]
                    = match;

                // Integer types.
                if ("diouxX".includes(type))
                {
                    let prefixUnsigned = true;
                    switch (length)
                    {
                        case "hh":
                            cType = "char";
                            break;
                        case "h":
                            cType = "short";
                            break;
                        case "l":
                            cType = "long";
                            break;
                        case "ll":
                            cType = "long long";
                            break;
                        case "j":
                            cType = ("uxX".includes(type)) ? "uintmax_t" : "intmax_t";
                            prefixUnsigned = false;
                            break;
                        case "z":
                            cType = "size_t";
                            prefixUnsigned = false;
                            break;
                        case "t":
                            cType = "ptrdiff_t";
                            prefixUnsigned = false;
                            break;
                        default:
                            cType = "int";
                            break;
                    }

                    if ("uxX".includes(type))
                    {
                        cType = "unsigned " + cType;
                    }
                }
                // Floating point types.
                else if ("fFeEgGaA".includes(type))
                {
                    if (length == "L")
                        cType = "long ";
                    cType += "double";
                }
                // String.
                else if (type == "s")
                {
                    let charType = "char";
                    if (g_wideChars || length == "l")
                        charType = "wchar_t";
                    cType = `const ${charType} *`;
                }
                // Character.
                else if (type == "c")
                {
                    if (g_wideChars || length == "l")
                        cType = "wchar_t";
                    else
                        cType = "char";
                }
                // Pointer.
                else if (type == "p")
                {
                    cType = "void *";
                }

                args.push({
                    name,
                    type: cType
                });
            }

            // Replace any loose % chars (including regular C formats like %s)
            // with %%
            let index = str.indexOf("%");
            while (index != -1)
            {
                let replace = true;
                for (const match of matches)
                {
                    if (index >= match.index && index <= match.index + match[0].length + 1)
                    {
                        replace = false;
                        break;
                    }
                }

                if (replace)
                {
                    str = str.slice(0, index) + "%" + str.slice(index);
                }
                index = str.indexOf("%", index + (replace ? 2 : 1));
            }

            result.formattedStrings[key] = {
                text: str.replaceAll(FORMATTED_STRING_REGEX, "%$2"),
                args
            };
        }
    }
    
    return result;
}

for (const record of g_recordList)
{
    let recordData = {};
    let defaultLangData = parseRecord(g_defaultLang, record);
    if (!defaultLangData)
        return;
    recordData[g_defaultLang] = defaultLangData;

    let defaultLangStrKeys = Object.keys(defaultLangData.strings);
    let defaultLangFormattedArgs = {};
    for (const key in defaultLangData.formattedStrings)
    {
        let args = defaultLangData.formattedStrings[key].args;
        defaultLangFormattedArgs[key] = args.map(a => `${a.name}:${a.type}`)
    }

    for (let i = 1; i < g_langList.length; i++)
    {
        let langData = parseRecord(g_langList[i], record);
        if (!langData)
            return null;

        if (defaultLangData.strings.length != langData.strings.length)
        {
            log(LogType.FATAL, `Number of strings in record '${record}' for '${g_langList[i]}' does not match that of '${g_defaultLang}'`);
            return;
        }

        if (defaultLangData.formattedStrings.length != langData.formattedStrings.length)
        {
            log(LogType.FATAL, `Number of formatted strings in record '${record}' for '${g_langList[i]}' does not match that of '${g_defaultLang}'`);
            return;
        }

        const containsAll = (arr1, arr2) => 
            arr2.every(arr2Item => arr1.includes(arr2Item))
            
        const equalMembers = (arr1, arr2) => 
                            containsAll(arr1, arr2) && containsAll(arr2, arr1);

        if (!equalMembers(Object.keys(langData.strings), defaultLangStrKeys))
        {
            log(LogType.FATAL, `String keys in record '${record}' for '${g_langList[i]}' does not match that of '${g_defaultLang}'`);
            return;
        }

        for (const key in langData.formattedStrings)
        {
            let args = langData.formattedStrings[key].args;
            let formattedArgs = args.map(a => `${a.name}:${a.type}`);
            if (!(key in defaultLangFormattedArgs))
            {
                log(LogType.FATAL, `Formatted string keys in record '${record}' for '${g_langList[i]}' does not match that of '${g_defaultLang}'`);
                return;
            }

            if (!equalMembers(formattedArgs, defaultLangFormattedArgs[key]))
            {
                log(LogType.FATAL, `Formatted string arguments for string '${key}' in record '${record}' for '${g_langList[i]}' does not match that of '${g_defaultLang}'`);
                return;
            }
        }
    }
}
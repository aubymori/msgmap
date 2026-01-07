const yaml = require("yaml");
const { argv } = require("node:process");



for (let i = 2; i < argv.length; i++)
{
    console.log(argv[i]);
}
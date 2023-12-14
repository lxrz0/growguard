const express = require("express");
const port = 3000; 
const app = express();
const l = console.log;
const path = require("path");
const mqtt = require("mqtt");
app.set("view engine", "ejs");
app.set("views", "public")
app.use("/public", express.static(path.join(__dirname, "public")))
app.use("/modules", express.static(path.join(__dirname, "node_modules"))) // expose modules folder

// create socket.io server
const http = require("http");
const server = http.createServer(app);
const io = require("socket.io")(server);
const mongoose = require("mongoose");
const SensorLog = require("./public/models/SensorLog");
const moment = require("moment/moment");
const TelegramBot = require("node-telegram-bot-api");
const PlantConfig = require("./public/models/PlantConfig");
const bodyParser = require("body-parser");

app.use(bodyParser({extended: false}));
app.use(bodyParser.json());

require("dotenv").config();

const sensorDataStorage = [];

// connect to database
mongoose.connect("mongodb://127.0.0.1:27017/sensor_history", {})


// connect to MQTT broker
const mqtt_broker = "23afd78a350347c689a20e4620b72ceb.s2.eu.hivemq.cloud";
const mqtt_sub_topic = "sensors/data";
const client = mqtt.connect({
    host: mqtt_broker,
    port: 8883, 
    protocol: "mqtts", 
    username: "cl586", 
    password: "cEu9NQfrRkvUmdYw6ZShn2" // put into env variable
});

/**
 * Method creates plant config if it does not exist
 */
const createPlantConfig = async _ => {
    try {
        const exists = await PlantConfig.countDocuments();
        if (exists == 1) return // do nothing

        // create new config with 0 values. Values need to be configured on the UI
        await new PlantConfig({
            temperature: 20, 
            soil_moisture: 50, 
            sunlight: 100,
        }).save();
    }
    
    catch (error) {
        console.log("(-) Failed to initialise plant config: ");
        console.log(error);    
    }
}

// setup telegram bot for notifications
const bot = new TelegramBot(process.env.TGBOT_KEY, {polling: true})

bot.on("message", msg => {
    if (["hello", "hi", "yo", "hey"].includes(msg.text)) {
        bot.sendMessage(msg.chat.id, "👋 Hello, I'm growguard. I'm your reporting assistant. Currently I'm only looking after Dave at the moment. Any changes in environment and I'll be the first to let you know!")
    }
})

const tg_user_id = "664169986"; // my telegram user id

// bot.sendMessage(tg_user_id, "🪴 Hello! I'm growguard. Looks like Dave needs to be watered")

async function main () {
    console.log("[+] initialising plant config")
    await createPlantConfig();
}

main();


/** method saves a json payload in memory - limited to 10 */
const saveInMemory = function (jsonPayload) {
    try {
        sensorDataStorage.push(jsonPayload);

        if (sensorDataStorage.length > 20) {
            // remove the first item from the array
            sensorDataStorage.shift();
        }
    } catch (error) {
        console.log(error);
    }
}

client.on("connect", _ => {
    console.log("[+] connected to mqtt broker");
    client.subscribe(mqtt_sub_topic, err => {
        if (!err) {
            l("[+] subscribed to topic", mqtt_sub_topic)
        } else {
            console.error("Failed to subscribe", err);
        }
    })
})

client.on("message", async (topic, data) => {
    l(Buffer.from(data).toString());

    let payload = {...JSON.parse(Buffer.from(data).toString()), timestamp: Date.now()};

    if (payload.deviation) {
        await bot.sendMessage(tg_user_id, `⚠️ Woah did something happen? - The ${payload.deviationProperty} sensor detected a sudden and abnormally large change in reading`)
    }

    if (payload.soil_digital == 1) {
        await bot.sendMessage(tg_user_id, `🪴 Looks like Dave needs to be watered`)
    }

    try {
        io.emit("event", {
            value: payload, 
            timestamp: Date.now(), 
        })
    } catch (error) {
        l(error.message)
    }

    // save to database
    await new SensorLog({data: payload}).save();
    saveInMemory(payload);
})

io.on("connection", s => {
    l(s.id, "connected");
})

app.get("/", (req, res) => {
    res.render("login")
});

app.get("/historical", async (req, res) => {
    // get previous weeks data 
    try {
        const weeksData = await SensorLog.find({createdAt: {$gte: moment().subtract(1, "week").valueOf()}});

        if (!weeksData || !weeksData.length) {
            return res.send({
                temperature: 0, 
                soil_moisture: 0, 
                air_humidity: 0, 
                sunlight: 0, 
            })
        }

        // aggregate all the values for each property and get the average
        let aggregateData = weeksData.reduce((all, log) => {
            // console.log(log.data);
            all.temperature += log.data.temperature;
            all.soil_moisture += log.data.soil_analog;
            all.air_humidity += log.data.humidity;
            all.sunlight += log.data.lightIntensity;

            return all;
        }, {
            temperature: 0, 
            soil_moisture: 0, 
            air_humidity: 0, 
            sunlight: 0, 
        });

        // get averages of all
        Object.keys(aggregateData).forEach(key => {
            aggregateData[key] = (aggregateData[key] / weeksData.length);
        });

        return res.send({
            weekAggregate: aggregateData, 
            weekRaw: weeksData
        });
    } 
    
    catch (error) {
        console.log(error);
        return res.status(500).send("internal server error")    
    }
});

app.post("/setplantconfig", async (req, res) => {
    try {

        console.log("hello!");
        console.log(req.body);

        // no form validation as i'm the only one with access currently
        const {temperature, moisture, sunlight} = req.body;

       // find config and update
       let c = await PlantConfig.findOne();
       c.temperature = temperature; 
       c.soil_moisture = moisture;
       c.sunlight = sunlight;

       await c.save();

       return res.status(200).send({success: true})

    } catch (error) {
        return res.status(500).send("Failed to process update")
    }
})

app.get("/correlation", async (req, res) => {
    try {
        // get soil moisture and temperature data for last 100 data points
        const data = await SensorLog.find().limit(1000);
        if (!data) {
            throw new Error ("could not find any data")
        }
        return res.send(data);
    } catch (error) {
        console.log(error);
        return res.status(500).send([])
    }
})

app.get("/plantconfig", async (req, res) => {
    try {
        let conf = await PlantConfig.findOne();
        return res.send(conf);
    } catch (error) {
        console.log(error);
        console.log("(-) failed to find plant config");

        return res.status(500).send({});
    }
})

app.get("/previousData", async (req, res) => {
    // retrieve from database if sensorDataStorage is empty
    const pastSensorData = await SensorLog.find().limit(20).lean();
    if (!pastSensorData.length) {
        return res.send(sensorDataStorage)
    }

    pastSensorData.map(p => {
        p.timestamp = p.createdAt
        Object.keys(p.data).forEach(key => {
            p[key] = p.data[key]
        })
    });

    console.log(pastSensorData)

    // send cached sensor data
    return res.send(pastSensorData);
})

app.get("/dashboard", (req, res) => {
    res.render("dashboard")
})

server.listen(port, _ => {
    l("server running on port", port);
})
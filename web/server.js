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

client.on("message", (topic, data) => {
    l(Buffer.from(data).toString());

    io.emit("event", {
        value: JSON.parse(Buffer.from(data).toString()), 
        timestamp: Date.now(), 
    })
})

io.on("connection", s => {
    l(s.id, "connected");
})

app.get("/", (req, res) => {
    res.render("login")
});

app.get("/dashboard", (req, res) => {
    res.render("dashboard")
})

server.listen(port, _ => {
    l("server running on port", port);
})
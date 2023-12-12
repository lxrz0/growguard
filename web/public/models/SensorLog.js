const mongoose = require("mongoose");
const SensorSchema = new mongoose.Schema({
    data: Object,
    createdAt: {
        type: Date, 
        default: Date.now(), 
        required: true, 
    }
})

module.exports = mongoose.model("SensorLog", SensorSchema);
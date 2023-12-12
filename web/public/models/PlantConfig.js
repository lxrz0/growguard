const mongoose = require("mongoose");
const PlantSchema = new mongoose.Schema({
    temperature: Number,
    soil_moisture: String, 
    sunlight: String,
    createdAt: {
        type: Date, 
        default: Date.now(), 
        required: true, 
    }
})

module.exports = mongoose.model("PlantConfig", PlantSchema);
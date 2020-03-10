angleConversions = require("angleConversions")
require("lla2ecef")

function convertLocationToDIS()

local lat = math.rad(DSimLocal.LocationInWorldCoordinates.X) --convert to radian
local lon = math.rad(DSimLocal.LocationInWorldCoordinates.Y)

--convert lat/long to geocentric
DSimLocal.LocationInWorldCoordinates.X, DSimLocal.LocationInWorldCoordinates.Y,DSimLocal.LocationInWorldCoordinates.Z = lla2ecef(DSimLocal.LocationInWorldCoordinates.X, DSimLocal.LocationInWorldCoordinates.Y,DSimLocal.LocationInWorldCoordinates.Z)

local yaw = DSimLocal.PDUData.Orientation.Psi --heading 
local pitch = DSimLocal.PDUData.Orientation.Theta --pitch 
local roll = DSimLocal.PDUData.Orientation.Phi --roll
end
angleConversions = require("angleConversions")
require("lla2ecef")

function convertLocationToHLA ()
local lat = math.rad(DSimLocal.DeadReckoningAlgorithm_A_Alternatives.WorldLocation.X)
local lon = math.rad(DSimLocal.DeadReckoningAlgorithm_A_Alternatives.WorldLocation.Y)

DSimLocal.DeadReckoningAlgorithm_A_Alternatives.WorldLocation.X,DSimLocal.DeadReckoningAlgorithm_A_Alternatives.WorldLocation.Y,DSimLocal.DeadReckoningAlgorithm_A_Alternatives.WorldLocation.Z = lla2ecef(DSimLocal.DeadReckoningAlgorithm_A_Alternatives.WorldLocation.X,DSimLocal.DeadReckoningAlgorithm_A_Alternatives.WorldLocation.Y,DSimLocal.DeadReckoningAlgorithm_A_Alternatives.WorldLocation.Z)

local yaw = DSimLocal.DeadReckoningAlgorithm_A_Alternatives.Orientation.Psi-- heading
local pitch = DSimLocal.DeadReckoningAlgorithm_A_Alternatives.Orientation.Theta--pitch
local roll = DSimLocal.DeadReckoningAlgorithm_A_Alternatives.Orientation.Phi--roll

DSimLocal.DeadReckoningAlgorithm_A_Alternatives.Orientation.Theta = (angleConversions.getThetaFromTaitBryanAngles(lat, lon, yaw, pitch));
DSimLocal.DeadReckoningAlgorithm_A_Alternatives.Orientation.Phi = (angleConversions.getPhiFromTaitBryanAngles(lat, lon, yaw, pitch, roll))
DSimLocal.DeadReckoningAlgorithm_A_Alternatives.Orientation.Psi =  (angleConversions.getPsiFromTaitBryanAngles(lat, lon, yaw, pitch)) 
end
angleConversions = require("angleConversions")
require("ecef2lla")

function convertLocationFromHLA ()
DSimLocal.DeadReckoningAlgorithm_A_Alternatives.WorldLocation.X,DSimLocal.DeadReckoningAlgorithm_A_Alternatives.WorldLocation.Y,DSimLocal.DeadReckoningAlgorithm_A_Alternatives.WorldLocation.Z = ecef2lla(DSimLocal.DeadReckoningAlgorithm_A_Alternatives.WorldLocation.X,DSimLocal.DeadReckoningAlgorithm_A_Alternatives.WorldLocation.Y,DSimLocal.DeadReckoningAlgorithm_A_Alternatives.WorldLocation.Z)

local lat = math.rad(DSimLocal.DeadReckoningAlgorithm_A_Alternatives.WorldLocation.X)  --converting to rad because function requires rad
local lon = math.rad(DSimLocal.DeadReckoningAlgorithm_A_Alternatives.WorldLocation.Y)

local psi =  DSimLocal.DeadReckoningAlgorithm_A_Alternatives.Orientation.Psi-- heading
local theta = DSimLocal.DeadReckoningAlgorithm_A_Alternatives.Orientation.Theta--pitch
local phi = DSimLocal.DeadReckoningAlgorithm_A_Alternatives.Orientation.Phi --roll

DSimLocal.DeadReckoningAlgorithm_A_Alternatives.Orientation.Psi =  angleConversions.getOrientationFromEuler(lat, lon, psi, theta)
DSimLocal.DeadReckoningAlgorithm_A_Alternatives.Orientation.Theta = angleConversions.getPitchFromEuler(lat, lon, psi, theta)
DSimLocal.DeadReckoningAlgorithm_A_Alternatives.Orientation.Phi = angleConversions.getRollFromEuler(lat, lon, psi, theta, phi)

end
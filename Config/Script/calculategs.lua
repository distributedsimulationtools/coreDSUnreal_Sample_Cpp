function calculategs ()
	if (DSimWorld ~= nil) and (DSimWorld.Aircraft1 ~= nil) then
		DSimLocal.Value=  tostring(math.sqrt(DSimWorld.Aircraft1.local_ax*DSimWorld.Aircraft1.local_ax + DSimWorld.Aircraft1.local_ay*DSimWorld.Aircraft1.local_ay+ DSimWorld.Aircraft1.local_az*DSimWorld.Aircraft1.local_az) / 9.8)
	end
end
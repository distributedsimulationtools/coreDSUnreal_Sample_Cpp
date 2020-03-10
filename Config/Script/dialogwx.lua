-------------------------------------------------------------------------=---
-- Name:        dialog.wx.lua
-- Purpose:     Dialog wxLua sample
--              Based on the C++ version by Marco Ghislanzoni
-- Author:      J Winwood, John Labenski
-- Created:     March 2002
-- Copyright:   (c) 2001 Lomtick Software. All rights reserved.
-- Licence:     wxWidgets licence
-------------------------------------------------------------------------=---

local dialog = {}

-- Load the wxLua module, does nothing if running from wxLua, wxLuaFreeze, or wxLuaEdit

package.cpath = package.cpath..";C:/Code/ForwardSim/HLAGenericAPI/trunk/API/Libraries/wxLua-2.8.12.3-Lua-5.2.2-MSW-Ansi/wxLua-2.8.12.3-Lua-5.2.2-MSW-Ansi/bin/?.dll;"
package.path = package.path..";C://Users//jpl//Desktop//DSIM Lua Script//?.lua"
-- Load the wxLua module, does nothing if running from wxLua, wxLuaFreeze, or wxLuaEdit
package.cpath = package.cpath..";./?.dll;./?.so;../lib/?.so;../lib/vc_dll/?.dll;../lib/bcc_dll/?.dll;../lib/mingw_dll/?.dll;"
require("wx")
require("convertDSIMLocalToString")



-- NOTES about validators!
-- The controls apparently must be in a wxDialog and they must also
-- be direct children of the dialog. You CANNOT put any controls that
-- you want to use wxGenericValidators with on a panel that's a child of
-- the dialog.

-- You cannot seem to set a wxGenericValidator to a control and then
-- check the value of the wxLuaObject that the validator uses at any
-- time either since the validator only updates it's value when the
-- function TransferDataFromWindow() is called. You cannot set a
-- wxGenericValidator to a wxCheckBox and then call TransferDataFromWindow()
-- on the validator to force it to update as this apparently does nothing.

-- Finally, it appears that you need to have an wxID_OK button for the
-- TransferDataTo/FromWindow() functions to be automatically called by the
-- dialog.

ID_TEST_VALIDATORS  = 1000
ID_CHECKBOX         = 1001
ID_COMBOBOX         = 1002
ID_TEXTCTRL         = 1003
ID_SCROLLBAR        = 1004
ID_CHECKLBOX        = 1005
ID_TEXTCTRL_TVAL_NUM = 1006

-- Set up the initial values for the validators, we use the wxLuaObjects
-- as proxies for the wxGenericValidators to pass the *int *bool, *string, etc,
-- pointers from the validators to and from to lua.

function dialog.update()
if (frame == nil) then
      frame = wx.wxFrame( wx.NULL,              -- no parent for toplevel windows
                          wx.wxID_ANY,          -- don't need a wxWindow ID
                          "Output from DSim", -- caption on the frame
                          wx.wxDefaultPosition, -- let system place the frame
                          wx.wxSize(450, 420),  -- set the size of the frame
                          wx.wxDEFAULT_FRAME_STYLE ) -- use default frame styles

      -- create a simple status bar
      frame:CreateStatusBar(1)
      frame:SetStatusText("Output from DSim")


      frameText = wx.wxTextCtrl(frame, wx.wxID_ANY, "Output from DSIM",
                                wx.wxDefaultPosition, wx.wxDefaultSize,
                                wx.wxTE_MULTILINE)


      -- show the frame window
      frame:Show(true)
     else
		--s = frameText:GetValue()  ;
		--s = s .. table.tostring( DSimLocal );

		frameText:SetValue(table.tostring( DSimLocal ));
     end
end

return dialog

-- Call wx.wxGetApp():MainLoop() last to start the wxWidgets event loop,
-- otherwise the wxLua program will exit immediately.
-- Does nothing if running from wxLua, wxLuaFreeze, or wxLuaEdit since the
-- MainLoop is already running or will be started by the C++ program.
--wx.wxGetApp():MainLoop()


# Makefile
#
# Automatically panelizes and exports JLCPCB fabrication files with KiKit.
# On Windows, be sure to invoke within the KiCad command prompt so KiKit can be found.
#
# Currently only works with Windows!

BOARD     := momentum

PANELDIR  := panel
OUTPUTDIR := fab

.PHONY: all clean web

${OUTPUTDIR}/pos.csv: ${PANELDIR}/${BOARD}_panel.kicad_pcb ${BOARD}.kicad_sch
	if not exist $(OUTPUTDIR) mkdir $(OUTPUTDIR)
	kikit fab jlcpcb --assembly --schematic ${BOARD}.kicad_sch ${PANELDIR}/${BOARD}_panel.kicad_pcb ${OUTPUTDIR}
    
${PANELDIR}/${BOARD}_panel.kicad_pcb: ${BOARD}.kicad_pcb
	if not exist $(PANELDIR) mkdir $(PANELDIR)
	kikit panelize \
	--layout "hspace: 2mm; vspace: 2mm; rows: 1; cols: 4" \
	--tabs "type: fixed; vwidth: 4mm; hwidth: 4mm; mindistance: 2mm; hcount: 2" ^\
	--cuts "type: mousebites; spacing: 1mm; offset: -0.25mm" \
	--framing "type: railstb; width: 3mm" \
	--text "type: simple; voffset: -1.5mm; layer: B.SilkS; text: JLCJLCJLCJLC; anchor: mb" \
	--copperfill "type: solid; clearance: 0.2mm" \
	${BOARD}.kicad_pcb ${PANELDIR}/${BOARD}_panel.kicad_pcb

panel: ${PANELDIR}/${BOARD}_panel.kicad_pcb

clean:
	-if exist $(PANELDIR) rmdir /s /q $(PANELDIR)
	-if exist $(OUTPUTDIR) rmdir /s /q $(OUTPUTDIR)
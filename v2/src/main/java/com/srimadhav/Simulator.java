package com.srimadhav;

import javax.swing.BorderFactory;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JToggleButton;
import javax.swing.JToolBar;
import javax.swing.SwingConstants;
import java.awt.Toolkit;
import java.awt.event.ComponentAdapter;
import java.awt.event.ComponentEvent;
import java.awt.CardLayout;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.GridLayout;
import java.awt.Color;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import javax.swing.Box;


import org.json.*;
import java.nio.file.*;

public class Simulator {
    static {
        System.loadLibrary("bridge");
    }
    public static native void run(String program,boolean isTrace);

    static int memsize, l1is, l1ib, l1ds, l1db, bsize;

    // --- class-level widget references so they can be updated after run ---
    static JLabel[] regs = new JLabel[32];
    static JTextArea resPanel  = new JTextArea();
    
    static JLabel[] memLabels;
    static JLabel[][][] l1iLabels;
    static JLabel[][][] l1dLabels;
    
    // UI Panel containers
    static JPanel memContentPanel;
    static JPanel l1iContentPanel;
    static JPanel l1dContentPanel;

    // --- helper: init cache UI ---
    static void initCacheUI(JPanel panel, JLabel[][][] labels, int sets, int blocks, int bsize) {
        panel.setLayout(new GridLayout(0, 1, 5, 5));
        for (int s = 0; s < sets; s++) {
            JPanel setPanel = new JPanel(new java.awt.BorderLayout());
            setPanel.setBackground(new Color(235, 255, 235)); // Light green
            setPanel.setBorder(BorderFactory.createTitledBorder(BorderFactory.createLineBorder(Color.GRAY), "Set " + s));
            
            JPanel blocksPanel = new JPanel(new GridLayout(0, 1, 2, 2));
            blocksPanel.setOpaque(false); // Let set color show between blocks
            
            for (int b = 0; b < blocks; b++) {
                int blockBase = (s * blocks + b) * bsize;
                JPanel blockPanel = new JPanel(new java.awt.BorderLayout());
                blockPanel.setBackground(new Color(215, 245, 215)); // Deeper green
                blockPanel.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));
                
                JLabel blockLabel = new JLabel(String.format("Block %d (base 0x%04X):", b, blockBase));
                blockLabel.setFont(new Font(Font.MONOSPACED, Font.BOLD, 12));
                blockLabel.setOpaque(true);
                blockLabel.setBackground(new Color(215, 245, 215)); // Match block color
                blockLabel.setBorder(BorderFactory.createCompoundBorder(
                    BorderFactory.createLineBorder(Color.GRAY),
                    BorderFactory.createEmptyBorder(5, 5, 5, 5)
                ));
                blockPanel.add(blockLabel, java.awt.BorderLayout.NORTH);
                
                JPanel bytesGrid = new JPanel(new GridLayout(0, 8, 0, 5)); // 8 bytes per row
                bytesGrid.setOpaque(false); // Let block color show between rows
                
                for (int i = 0; i < bsize; i++) {
                    JLabel byteLabel = new JLabel("00", SwingConstants.CENTER);
                    byteLabel.setFont(new Font(Font.MONOSPACED, Font.PLAIN, 12));
                    byteLabel.setBorder(BorderFactory.createCompoundBorder(
                        BorderFactory.createLineBorder(Color.GRAY),
                        BorderFactory.createEmptyBorder(5, 5, 5, 5)
                    ));
                    
                    byteLabel.setBackground(new Color(195, 235, 195)); // Uniform slightly darker green
                    byteLabel.setOpaque(true);
                    
                    byteLabel.setPreferredSize(new Dimension(40, 30));
                    labels[s][b][i] = byteLabel;
                    bytesGrid.add(byteLabel);
                }
                blockPanel.add(bytesGrid, java.awt.BorderLayout.CENTER);
                blocksPanel.add(blockPanel);
            }
            setPanel.add(blocksPanel, java.awt.BorderLayout.CENTER);
            panel.add(setPanel);
        }
    }

    public static void main(String[] args) {

        try {
            String content = new String(Files.readAllBytes(Paths.get("config.json")));
            JSONObject json = new JSONObject(content);
            bsize = json.getInt("bsize");
            memsize = json.getInt("memory");
            l1is = json.getInt("sets1");
            l1ib = json.getInt("l1i") / (l1is * bsize);
            l1ds = json.getInt("sets1");
            l1db = json.getInt("l1d") / (l1ds * bsize);
        } catch (Exception e) {
            System.out.println("Config parsing error\t" + e);
        }

        JFrame window = new JFrame("RISC-V Simulator");
        Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
        int screenWidth  = screenSize.width;
        int screenHeight = screenSize.height;
        window.setSize(screenWidth, screenHeight);
        window.setLayout(null);

        CardLayout pLayout  = new CardLayout();
        JPanel mainPanel    = new JPanel(pLayout);

        JButton b1 = new JButton("Editor");
        JButton b2 = new JButton("Registers");
        JButton b3 = new JButton("Memory");
        JButton b4 = new JButton("Cache");

        JButton[] navBtns = {b1, b2, b3, b4};
        for (JButton btn : navBtns) {
            btn.setFocusable(false);
            btn.setBackground(new Color(225, 225, 235));
            btn.setBorder(BorderFactory.createCompoundBorder(
                BorderFactory.createLineBorder(Color.GRAY),
                BorderFactory.createEmptyBorder(4, 12, 4, 12)
            ));
        }

        JPanel execStatsPanel = new JPanel(new GridLayout(0, 1, 0, 2));
        execStatsPanel.setBorder(BorderFactory.createTitledBorder(BorderFactory.createLineBorder(Color.GRAY), "Execution Stats"));
        execStatsPanel.setBackground(new Color(245, 245, 250));
        
        JLabel clockCyclesLbl = new JLabel(" Clock Cycles: 0");
        JLabel exStallsLbl    = new JLabel(" Ex Stalls: 0");
        JLabel memStallsLbl   = new JLabel(" Mem Stalls: 0");
        JLabel instrLbl       = new JLabel(" Instructions: 0");
        JLabel ipcLbl         = new JLabel(" IPC: 0.0");
        
        JLabel[] execLabels = {clockCyclesLbl, exStallsLbl, memStallsLbl, instrLbl, ipcLbl};
        for (JLabel lbl : execLabels) {
            lbl.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createLineBorder(Color.LIGHT_GRAY), BorderFactory.createEmptyBorder(2, 5, 2, 5)));
            lbl.setOpaque(true);
            lbl.setBackground(Color.WHITE);
            execStatsPanel.add(lbl);
        }

        JPanel mmuStatsPanel = new JPanel(new GridLayout(0, 1, 0, 2));
        mmuStatsPanel.setBorder(BorderFactory.createTitledBorder(BorderFactory.createLineBorder(Color.GRAY), "MMU Stats"));
        mmuStatsPanel.setBackground(new Color(250, 245, 245));
        
        JLabel tlbHitsLbl     = new JLabel(" TLB Hits: 0");
        JLabel tlbMissesLbl   = new JLabel(" TLB Misses: 0");
        JLabel pageFaultsLbl  = new JLabel(" Page Faults: 0");
        JLabel evictionsLbl   = new JLabel(" Evictions: 0");
        JLabel writebacksLbl  = new JLabel(" Dirty WBs: 0");
        JLabel swapOutsLbl    = new JLabel(" Swap Outs: 0");
        JLabel swapInsLbl     = new JLabel(" Swap Ins: 0");
        JLabel transPenaltyLbl= new JLabel(" Trans Penalty: 0");

        JLabel[] mmuLabels = {tlbHitsLbl, tlbMissesLbl, pageFaultsLbl, evictionsLbl, writebacksLbl, swapOutsLbl, swapInsLbl, transPenaltyLbl};
        for (JLabel lbl : mmuLabels) {
            lbl.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createLineBorder(Color.LIGHT_GRAY), BorderFactory.createEmptyBorder(2, 5, 2, 5)));
            lbl.setOpaque(true);
            lbl.setBackground(Color.WHITE);
            mmuStatsPanel.add(lbl);
        }


        // ── Program panel ──────────────────────────────────────────────────
        JPanel program  = new JPanel();
        program.setLayout(null);
        JTextArea assembly = new JTextArea();
        JToolBar  top      = new JToolBar();
        JScrollPane editor = new JScrollPane(assembly);

        resPanel.setEditable(false);
        resPanel.setFont(new Font(Font.MONOSPACED, Font.PLAIN, 12));

        JButton runBtn   = new JButton("Run");
        JButton clRes    = new JButton("Clear Output");
        JButton clProg   = new JButton("Clear Program");
        JButton loadProg = new JButton("Load Program");
        JButton saveProg = new JButton("Save Program");
        JToggleButton isTrace = new JToggleButton("OFF"){
            @Override
            protected void paintComponent(Graphics g) {
                Graphics2D g2 = (Graphics2D) g.create();
                g2.setRenderingHint(
                    RenderingHints.KEY_ANTIALIASING,
                    RenderingHints.VALUE_ANTIALIAS_ON
                );
                if (isSelected())
                    g2.setColor(Color.GREEN);
                else
                    g2.setColor(Color.LIGHT_GRAY);
                g2.fillRoundRect(
                    0, 0,
                    getWidth(), getHeight(),
                    getHeight(), getHeight()
                );
                int diameter = getHeight() - 4;
                int x = isSelected()? getWidth() - diameter - 2: 2;
                g2.setColor(Color.WHITE);
                g2.fillOval(x, 2, diameter, diameter);
                g2.dispose();
            }
        };
        isTrace.setBorderPainted(false);

        JButton[] edBtns = {runBtn, clRes, clProg, loadProg, saveProg};
        for (JButton btn : edBtns) {
            btn.setFocusable(false);
            btn.setBackground(new Color(225, 225, 235));
            btn.setBorder(BorderFactory.createCompoundBorder(
                BorderFactory.createLineBorder(Color.GRAY),
                BorderFactory.createEmptyBorder(4, 12, 4, 12)
            ));
        }

        clProg.addActionListener(e -> assembly.setText(""));
        clRes.addActionListener(e  -> resPanel.setText(""));
        loadProg.addActionListener(e -> {
            javax.swing.JFileChooser fc = new javax.swing.JFileChooser();
            if (fc.showOpenDialog(window) == javax.swing.JFileChooser.APPROVE_OPTION) {
                try {
                    assembly.setText(new String(Files.readAllBytes(fc.getSelectedFile().toPath())));
                } catch (Exception ex) {
                    resPanel.append("Error reading file: " + ex.getMessage() + "\n");
                }
            }
        });
        saveProg.addActionListener(e -> {
            javax.swing.JFileChooser fc = new javax.swing.JFileChooser();
            if (fc.showSaveDialog(window) == javax.swing.JFileChooser.APPROVE_OPTION) {
                try {
                    Files.write(fc.getSelectedFile().toPath(), assembly.getText().getBytes());
                    resPanel.append("Saved successfully to " + fc.getSelectedFile().getName() + "\n");
                } catch (Exception ex) {
                    resPanel.append("Error saving file: " + ex.getMessage() + "\n");
                }
            }
        });

        top.add(runBtn);
        top.add(clRes);
        top.add(clProg);
        top.add(loadProg);
        top.add(saveProg);
        top.add(Box.createHorizontalGlue());
        top.add(new JLabel("Trace Execution: "));
        top.add(isTrace);
        program.add(top);
        program.add(editor);
        program.add(resPanel);

        // ── Registers panel ────────────────────────────────────────────────
        JPanel registersPanel = new JPanel();
        registersPanel.setLayout(null);
        JPanel gridPanel = new JPanel(new GridLayout(16, 2, 10, 10));
        gridPanel.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
        for (int i = 0; i < 16; i++) {
            // Left Cell (x_i)
            JPanel cell1 = new JPanel(new GridLayout(1, 2, 0, 0));
            cell1.setBorder(BorderFactory.createLineBorder(Color.GRAY));
            cell1.setPreferredSize(new Dimension(200, 50));
            
            JLabel lbl1 = new JLabel("x" + i, SwingConstants.CENTER);
            lbl1.setFont(new Font(Font.MONOSPACED, Font.BOLD, 18));
            lbl1.setBorder(BorderFactory.createMatteBorder(0, 0, 0, 1, Color.GRAY));
            lbl1.setBackground(new Color(235, 245, 255)); // Light cyan/blue
            lbl1.setOpaque(true);
            
            regs[i] = new JLabel("0", SwingConstants.CENTER);
            regs[i].setFont(new Font(Font.MONOSPACED, Font.BOLD, 18));
            regs[i].setBackground(new Color(215, 235, 255)); // Darker tinge
            regs[i].setOpaque(true);
            
            cell1.add(lbl1);
            cell1.add(regs[i]);
            gridPanel.add(cell1);
            
            // Right Cell (x_{i+16})
            JPanel cell2 = new JPanel(new GridLayout(1, 2, 0, 0));
            cell2.setBorder(BorderFactory.createLineBorder(Color.GRAY));
            cell2.setPreferredSize(new Dimension(200, 50));
            
            JLabel lbl2 = new JLabel("x" + (i + 16), SwingConstants.CENTER);
            lbl2.setFont(new Font(Font.MONOSPACED, Font.BOLD, 18));
            lbl2.setBorder(BorderFactory.createMatteBorder(0, 0, 0, 1, Color.GRAY));
            lbl2.setBackground(new Color(235, 245, 255)); // Light cyan/blue
            lbl2.setOpaque(true);
            
            regs[i + 16] = new JLabel("0", SwingConstants.CENTER);
            regs[i + 16].setFont(new Font(Font.MONOSPACED, Font.BOLD, 18));
            regs[i + 16].setBackground(new Color(215, 235, 255)); // Darker tinge
            regs[i + 16].setOpaque(true);
            
            cell2.add(lbl2);
            cell2.add(regs[i + 16]);
            gridPanel.add(cell2);
        }
        JScrollPane regPane = new JScrollPane(gridPanel);
        regPane.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
        regPane.getVerticalScrollBar().setUnitIncrement(16); // solve scrollbar issues
        registersPanel.add(regPane);

        // ── Memory panel ───────────────────────────────────────────────────
        JPanel memPanel = new JPanel();
        memPanel.setLayout(null);
        
        memContentPanel = new JPanel(new GridLayout(0, 5, 0, 10)); // Addr + 4 Bytes
        memContentPanel.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
        memLabels = new JLabel[memsize];
        
        // Headers
        JLabel addrHdr = new JLabel("Address", SwingConstants.CENTER);
        addrHdr.setFont(new Font(Font.MONOSPACED, Font.BOLD, 14));
        addrHdr.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createLineBorder(Color.GRAY), BorderFactory.createEmptyBorder(5, 5, 5, 5)));
        memContentPanel.add(addrHdr);
        for(int i=0; i<4; i++) {
            JLabel bHdr = new JLabel("+0x" + i, SwingConstants.CENTER);
            bHdr.setFont(new Font(Font.MONOSPACED, Font.BOLD, 14));
            bHdr.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createLineBorder(Color.GRAY), BorderFactory.createEmptyBorder(5, 5, 5, 5)));
            memContentPanel.add(bHdr);
        }
        
        int displayMemSize = Math.min(memsize, 4096);
        for (int i = 0; i < displayMemSize; i += 4) {
            JLabel addrLabel = new JLabel(String.format("0x%08X", i), SwingConstants.CENTER);
            addrLabel.setFont(new Font(Font.MONOSPACED, Font.BOLD, 12));
            addrLabel.setForeground(Color.BLUE);
            addrLabel.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createLineBorder(Color.GRAY), BorderFactory.createEmptyBorder(5, 5, 5, 5)));
            addrLabel.setBackground(new Color(255, 250, 230)); // Light yellow
            addrLabel.setOpaque(true);
            memContentPanel.add(addrLabel);
            
            for (int j = 0; j < 4; j++) {
                if (i + j < displayMemSize) {
                    JLabel valLabel = new JLabel("00", SwingConstants.CENTER);
                    valLabel.setFont(new Font(Font.MONOSPACED, Font.PLAIN, 12));
                    valLabel.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createLineBorder(Color.GRAY), BorderFactory.createEmptyBorder(5, 5, 5, 5)));
                    
                    // increase the tinge for each byte cell
                    int shade = 250 - (j * 15);
                    valLabel.setBackground(new Color(255, shade, 200 - (j * 10)));
                    valLabel.setOpaque(true);
                    
                    memLabels[i + j] = valLabel;
                    memContentPanel.add(valLabel);
                } else {
                    memContentPanel.add(new JLabel("")); // padding if not multiple of 4
                }
            }
        }

        JScrollPane memPane = new JScrollPane(memContentPanel);
        memPane.getVerticalScrollBar().setUnitIncrement(16);
        memPanel.add(memPane);

        // ── Cache panel (nested CardLayout: L1I / L1D / L2) ───────────────
        JPanel cachePanel = new JPanel();
        cachePanel.setLayout(null);

        JToolBar cacheBar       = new JToolBar();
        JButton  cbl1i          = new JButton("L1I");
        JButton  cbl1d          = new JButton("L1D");

        JButton[] cacheBtns = {cbl1i, cbl1d};
        for (JButton btn : cacheBtns) {
            btn.setFocusable(false);
            btn.setBackground(new Color(225, 225, 235));
            btn.setBorder(BorderFactory.createCompoundBorder(
                BorderFactory.createLineBorder(Color.GRAY),
                BorderFactory.createEmptyBorder(4, 12, 4, 12)
            ));
        }

        cacheBar.add(cbl1i);
        cacheBar.add(cbl1d);

        CardLayout cacheLayout = new CardLayout();
        JPanel     cacheCards  = new JPanel(cacheLayout);

        l1iContentPanel = new JPanel();
        l1dContentPanel = new JPanel();
        
        // Add minimal padding
        l1iContentPanel.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
        l1dContentPanel.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));

        l1iLabels = new JLabel[l1is][l1ib][bsize];
        l1dLabels = new JLabel[l1ds][l1db][bsize];
        
        initCacheUI(l1iContentPanel, l1iLabels, l1is, l1ib, bsize);
        initCacheUI(l1dContentPanel, l1dLabels, l1ds, l1db, bsize);

        JScrollPane l1iPane = new JScrollPane(l1iContentPanel);
        JScrollPane l1dPane = new JScrollPane(l1dContentPanel);
        
        l1iPane.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
        l1dPane.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
        
        l1iPane.getVerticalScrollBar().setUnitIncrement(16);
        l1dPane.getVerticalScrollBar().setUnitIncrement(16);

        cacheCards.add(l1iPane, "L1I");
        cacheCards.add(l1dPane, "L1D");

        cbl1i.addActionListener(e -> { cacheLayout.show(cacheCards, "L1I"); cacheCards.revalidate(); cacheCards.repaint(); });
        cbl1d.addActionListener(e -> { cacheLayout.show(cacheCards, "L1D"); cacheCards.revalidate(); cacheCards.repaint(); });

        cachePanel.add(cacheBar);
        cachePanel.add(cacheCards);

        // ── Wire main nav buttons ──────────────────────────────────────────
        mainPanel.add(program,        "Program");
        mainPanel.add(registersPanel, "Registers");
        mainPanel.add(memPanel,       "Memory");
        mainPanel.add(cachePanel,     "Cache");

        b1.addActionListener(e -> { pLayout.show(mainPanel, "Program");   mainPanel.revalidate(); mainPanel.repaint(); });
        b2.addActionListener(e -> { pLayout.show(mainPanel, "Registers"); mainPanel.revalidate(); mainPanel.repaint(); });
        b3.addActionListener(e -> { pLayout.show(mainPanel, "Memory");    mainPanel.revalidate(); mainPanel.repaint(); });
        b4.addActionListener(e -> { pLayout.show(mainPanel, "Cache");     mainPanel.revalidate(); mainPanel.repaint(); });

        window.add(b1);
        window.add(b2);
        window.add(b3);
        window.add(b4);
        window.add(execStatsPanel);
        window.add(mmuStatsPanel);
        window.add(mainPanel);

        // ── Layout lambda (runs on resize + first paint) ───────────────────
        Runnable applyLayout = () -> {
            int w = window.getContentPane().getWidth();
            int h = window.getContentPane().getHeight();

            mainPanel.setBounds(0, 0, 23 * w / 25, h);
            b1.setBounds(23 * w / 25, 0,            w - 23 * w / 25, h / 15);
            b2.setBounds(23 * w / 25, h / 15,       w - 23 * w / 25, h / 15);
            b3.setBounds(23 * w / 25, 2 * h / 15,   w - 23 * w / 25, h / 15);
            b4.setBounds(23 * w / 25, 3 * h / 15,   w - 23 * w / 25, h / 15);
            
            int yOffset = 4 * h / 15 + h / 30; // gap below b4
            int execHeight = 7 * (h / 30);
            execStatsPanel.setBounds(23 * w / 25, yOffset, w - 23 * w / 25, execHeight);
            
            yOffset += execHeight + h / 30; // gap below execStats
            int mmuHeight = 10 * (h / 30);
            mmuStatsPanel.setBounds(23 * w / 25, yOffset, w - 23 * w / 25, mmuHeight);

            // program panel
            top.setBounds(0, 0, 23 * w / 25, h / 20);
            editor.setBounds(0, h / 20, 23 * w / 25, 3 * h / 4);
            resPanel.setBounds(0, h / 20 + 3 * h / 4, 23 * w / 25, h - 3 * h / 4 - h / 20);

            // registers panel
            regPane.setBounds(0, 0, 23 * w / 25, h);

            // memory panel
            memPane.setBounds(0, 0, 23 * w / 25, h);

            // cache panel
            int cacheBarH = h / 20;
            cacheBar.setBounds(0, 0, 23 * w / 25, cacheBarH);
            cacheCards.setBounds(0, cacheBarH, 23 * w / 25, h - cacheBarH);

            mainPanel.revalidate();
            mainPanel.repaint();
        };

        window.addComponentListener(new ComponentAdapter() {
            @Override
            public void componentResized(ComponentEvent e) {
                applyLayout.run();
            }
        });

        // ── Run button: call native, parse hardware.json, refresh all views ─
        runBtn.addActionListener(e -> {
            run(assembly.getText(),isTrace.isSelected());
            try {
                String result = new String(Files.readAllBytes(Paths.get("hardware.json")));
                JSONObject json = new JSONObject(result);

                // result summary
                int   clocks    = json.getJSONObject("result").getInt("clocks");
                int   instnum   = json.getJSONObject("result").getInt("numinsts");
                int   exstalls  = json.getJSONObject("result").getInt("exstalls");
                int   memstalls = json.getJSONObject("result").getInt("memstalls");
                float ipc       = json.getJSONObject("result").getFloat("ipc");
                resPanel.append("Clock Cycles=" + clocks + ", Ex Stalls=" + exstalls
                    + ", Mem Stalls=" + memstalls + ", Number of Instructions=" + instnum
                    + ", IPC=" + ipc + "\n");
                
                clockCyclesLbl.setText(" Clock Cycles: " + clocks);
                exStallsLbl.setText(" Ex Stalls: " + exstalls);
                memStallsLbl.setText(" Mem Stalls: " + memstalls);
                instrLbl.setText(" Instructions: " + instnum);
                ipcLbl.setText(" IPC: " + ipc);
                
                // VM Stats
                if(json.has("mmu")) {
                    JSONObject mmu = json.getJSONObject("mmu");
                    int tlbHits = mmu.getInt("tlbHits");
                    int tlbMisses = mmu.getInt("tlbMisses");
                    int pageFaults = mmu.getInt("pageFaults");
                    int evictions = mmu.getInt("evictions");
                    int dirtyWritebacks = mmu.getInt("dirtyWritebacks");
                    int swapOuts = mmu.getInt("swapOuts");
                    int swapIns = mmu.getInt("swapIns");
                    int totalTranslationPenalty = mmu.getInt("totalTranslationPenalty");

                    resPanel.append("--- VM Stats ---\n");
                    resPanel.append("TLB Hits=" + tlbHits + ", TLB Misses=" + tlbMisses + ", Page Faults=" + pageFaults + "\n");
                    resPanel.append("Evictions=" + evictions + ", Dirty WBs=" + dirtyWritebacks + "\n");
                    resPanel.append("Swap Outs=" + swapOuts + ", Swap Ins=" + swapIns + "\n");
                    resPanel.append("Total Translation Penalty=" + totalTranslationPenalty + " cycles\n\n");
                    
                    tlbHitsLbl.setText(" TLB Hits: " + tlbHits);
                    tlbMissesLbl.setText(" TLB Misses: " + tlbMisses);
                    pageFaultsLbl.setText(" Page Faults: " + pageFaults);
                    evictionsLbl.setText(" Evictions: " + evictions);
                    writebacksLbl.setText(" Dirty WBs: " + dirtyWritebacks);
                    swapOutsLbl.setText(" Swap Outs: " + swapOuts);
                    swapInsLbl.setText(" Swap Ins: " + swapIns);
                    transPenaltyLbl.setText(" Trans Penalty: " + totalTranslationPenalty);
                } else {
                    resPanel.append("\n");
                }

                // registers
                JSONArray regarr = json.getJSONArray("registers");
                for (int i = 0; i < 32; i++) {
                    regs[i].setText(String.valueOf(regarr.getInt(i)));
                }

                // memory
                JSONArray memarr = json.getJSONArray("memory");
                int currentDisplayMemSize = Math.min(memsize, 4096);
                for (int i = 0; i < currentDisplayMemSize; i++) {
                    memLabels[i].setText(String.format("%02X", memarr.getInt(i)));
                }

                // l1d
                JSONObject l1dJson  = json.getJSONObject("l1d");
                JSONArray  l1dSets  = l1dJson.getJSONArray("sets");
                for (int s = 0; s < l1ds; s++) {
                    JSONArray set = l1dSets.getJSONArray(s);
                    for (int b = 0; b < l1db; b++) {
                        JSONArray bytes = set.getJSONObject(b).getJSONArray("bytes");
                        for (int i = 0; i < bsize; i++) {
                            l1dLabels[s][b][i].setText(String.format("%02X", bytes.getInt(i)));
                        }
                    }
                }

                // l1i
                JSONObject l1iJson  = json.getJSONObject("l1i");
                JSONArray  l1iSets  = l1iJson.getJSONArray("sets");
                for (int s = 0; s < l1is; s++) {
                    JSONArray set = l1iSets.getJSONArray(s);
                    for (int b = 0; b < l1ib; b++) {
                        JSONArray bytes = set.getJSONObject(b).getJSONArray("bytes");
                        for (int i = 0; i < bsize; i++) {
                            l1iLabels[s][b][i].setText(String.format("%02X", bytes.getInt(i)));
                        }
                    }
                }



            } catch (Exception ex) {
      resPanel.append("Error reading hardware.json: "+ex.getMessage()+"\n");}});

    window.setVisible(true);window.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);applyLayout.run();}

    
    
}
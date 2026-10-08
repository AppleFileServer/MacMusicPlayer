/* MacMusic's Finder icons (Fix 1, 2026-10-08): the application, a 4G iPod
   front (the user's 32x32 design), and its song files (type iPdS), a
   document with a note. The library marker (iPdL) and the prefs file keep
   the Finder's generic icons (K1). The Finder uses these only if the
   application's "has bundle" Finder flag is set, which
   tools/set-bundle-bit.py does after each build (K2). */

#include "Finder.r"
#include "Icons.r"

/* The signature resource: MacMusic's creator code. */
data 'iPdM' (0, "Owner resource") {
    $"0C" "MacMusic 0.1"
};

resource 'BNDL' (128) {
    'iPdM', 0,
    {
        'ICN#', { 0, 128; 1, 129 };
        'FREF', { 0, 128; 1, 129 }
    }
};

resource 'FREF' (128) { 'APPL', 0, "" };
resource 'FREF' (129) { 'iPdS', 1, "" };

/* The application: a 4G iPod front. */
/*
           ################        
          #                #       
         #                  #      
         #  ##############  #      
         #  #            #  #      
         #  #       ##   #  #      
         #  #    ### #   #  #      
         #  #    #   #   #  #      
         #  #    #  ##   #  #      
         #  #   ##  ##   #  #      
         #  #   ##       #  #      
         #  #            #  #      
         #  ##############  #      
         #                  #      
         #       ####       #      
         #     ##    ##     #      
         #    #        #    #      
         #   #          #   #      
         #  #            #  #      
         #  #     ##     #  #      
         # #     #  #     # #      
         # #    #    #    # #      
         # #    #    #    # #      
         # #     #  #     # #      
         #  #     ##     #  #      
         #  #            #  #      
         #   #          #   #      
         #    #        #    #      
         #     ##    ##     #      
         #       ####       #      
          #                #       
           ################        
*/
resource 'ICN#' (128, "MacMusic") {
    {
        /* icon */
        $"00FFFF00 01000080 02000040 027FFE40 02400240 02406240 0243A240 02422240"
        $"02426240 02466240 02460240 02400240 027FFE40 02000040 0203C040 020C3040"
        $"02100840 02200440 02400240 02418240 02824140 02842140 02842140 02824140"
        $"02418240 02400240 02200440 02100840 020C3040 0203C040 01000080 00FFFF00",
        /* mask */
        $"00FFFF00 01FFFF80 03FFFFC0 03FFFFC0 03FFFFC0 03FFFFC0 03FFFFC0 03FFFFC0"
        $"03FFFFC0 03FFFFC0 03FFFFC0 03FFFFC0 03FFFFC0 03FFFFC0 03FFFFC0 03FFFFC0"
        $"03FFFFC0 03FFFFC0 03FFFFC0 03FFFFC0 03FFFFC0 03FFFFC0 03FFFFC0 03FFFFC0"
        $"03FFFFC0 03FFFFC0 03FFFFC0 03FFFFC0 03FFFFC0 03FFFFC0 01FFFF80 00FFFF00"
    }
};

/* Song files: a document with a note. */
/*
       #################           
       #               ##          
       #               # #         
       #               #  #        
       #               #   #       
       #               #    #      
       #               #     #     
       #               ########    
       #                      #    
       #                      #    
       #                      #    
       #                      #    
       #      ############    #    
       #      ############    #    
       #      #          #    #    
       #      #          #    #    
       #      #          #    #    
       #      #          #    #    
       #      #          #    #    
       #      #          #    #    
       #      #          #    #    
       #      #          #    #    
       #      #          #    #    
       #    ###        ###    #    
       #   ####       ####    #    
       #  ####       ####     #    
       #   ##         ##      #    
       #                      #    
       #                      #    
       #                      #    
       #                      #    
       ########################    
*/
resource 'ICN#' (129, "Song") {
    {
        /* icon */
        $"0FFFF800 08000C00 08000A00 08000900 08000880 08000840 08000820 08000FF0"
        $"08000010 08000010 08000010 08000010 081FFE10 081FFE10 08100210 08100210"
        $"08100210 08100210 08100210 08100210 08100210 08100210 08100210 08700E10"
        $"08F01E10 09E03C10 08C01810 08000010 08000010 08000010 08000010 0FFFFFF0",
        /* mask */
        $"0FFFF800 0FFFFC00 0FFFFE00 0FFFFF00 0FFFFF80 0FFFFFC0 0FFFFFE0 0FFFFFF0"
        $"0FFFFFF0 0FFFFFF0 0FFFFFF0 0FFFFFF0 0FFFFFF0 0FFFFFF0 0FFFFFF0 0FFFFFF0"
        $"0FFFFFF0 0FFFFFF0 0FFFFFF0 0FFFFFF0 0FFFFFF0 0FFFFFF0 0FFFFFF0 0FFFFFF0"
        $"0FFFFFF0 0FFFFFF0 0FFFFFF0 0FFFFFF0 0FFFFFF0 0FFFFFF0 0FFFFFF0 0FFFFFF0"
    }
};

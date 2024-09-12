/*
    SHAPES
*/
module star(inner_diameter, outer_diameter, steps, height) {
    r1 = outer_diameter / 2;
    r2 = inner_diameter / 2;
    linear_extrude(height = height) 
        polygon(points=[for (i = [0:2*steps-1]) 
            let (angle = i * 360 / (2*steps)) 
            if (i % 2 == 0) 
                [r1*cos(angle), r1*sin(angle)] 
            else 
                [r2*cos(angle), r2*sin(angle)]
        ]);
}

module cylinder_extrude($fn, r, h, scale = 1) {
    linear_extrude(height = h, scale = scale)
    polygon(points=[for (i = [0:$fn-1])
        let (angle = i * 360 / $fn)
        [r*cos(angle), r*sin(angle)]
    ]);
}

module circular(diameter, steps, height) {
    cylinder_extrude($fn = steps, r = diameter / 2, h = height);
}

module donut(inner_diameter, outer_diameter, steps, height) {
  difference() {
    circular(outer_diameter, steps, height);
    circular(inner_diameter, steps, height);
  }  
}

/*
    SPLINES
*/

module spline(steps = 20, di = 8.5, do = 6.5, ci = 0.6, co = 0.6, height = 10) {
    union() {
        difference() {
            star(di, do, steps, height);
            rotate(a = 360 / steps / 2) {
                donut(do - co, do + 100, steps, height);
            }
        }
        
        circular(di + ci, steps, height);
    }
}

module big_spline(height) {
    steps = 63;
    
    // between PLA and METAL:
    // 0.3mm outer diameter difference (PLA-, METAL+) 22.6, 22.9
    // 0.2mm inner diameter difference (PLA-, METAL+) 22.4, 22.6
    
    // both lowered 0.3mm (final: 22.8 to 22.5)
    
    // di lowered an additional 0.3mm
    // di = 21.5;
    //di = 20.9;
    //di = 21.2;
    di = 21.0;
    
    // do = 23.5;
    //do = 23.2;
    //do = 23.5;
    do = 23.3;
    
    // cutoff
    ci = 0.6;
    co = 0.6;
    
    spline(steps, di, do, ci, co, height);
}

module small_spline(height) {
    steps = 33;
    
    // between PLA and METAL:
    // 0.1mm outer difference (PLA-, METAL+)

    
    // with inner 14.7, outer 16.7:
    // 15.2 measured inner diameter
    // 15.9 measured outer diameter
    
    // todo:
    // increase hole by 0.3mm (new measured: 15.5 inner, 16.2 outer)
    // make outer flatter (spline teeth are flat)
    
    // both increased 0.3mm (final: 16.1 to 16.4)
    // do increased by additional 0.3mm
    
    //di = 14.7;
    //di = 14.8; // was 14.7
    di = 14.8;
    
    // do = 16.7;
    //do = 17.1; // was 17.0
    do = 17.1;
    
    // cutoff
    ci = 0.6;
    co = 0.6;
    
    spline(steps, di, do, ci, co, height);
}

/*
    ADAPTER

    NOTES TO CONSIDER:

    Spacing
    - The wheel needs spacing (at least 5mm) from the flat plastic (in line with the clockspring)
    - Currently, the spacing is 0mm (scrapes the plastic)
    
    Gradients
*/

module small_shaft() {
    spline_offset = 15;
    allowance = 0.1 + 0.1; // 0.4 lines up with the inner spline radius
    
    // cylinder 1 (above inner spline) dt = 16, db = 16
    //c1_dt = 15.0;
    //c1_db = 15.0;
    //c1_offset = 22;
    
    // cylinder 2 (below inner spline) dt = 17, db = 17
    c2_dt = 17;
    c2_db = 17;
    c2_offset = 10;
    
    // cylinder 3 (below cylinder 2) dt = 17, db = 19
    c3_dt = 17;
    c3_db = 19; // old: 19 (+ 0.2 for extra 1mm)
    c3_offset = 0;
    
    // radius calculation
    //c1_rt = (c1_dt + allowance) / 2;
    //c1_rb = (c1_db + allowance) / 2;
    c2_rt = (c2_dt + allowance) / 2;
    c2_rb = (c2_db + allowance) / 2;
    c3_rt = (c3_dt + allowance) / 2;
    c3_rb = (c3_db + allowance) / 2;
    
    //translate([0, 0, c1_offset]) rotate(a = 360 / 33 / 2) cylinder_extrude($fn = 33, h = 8, r = c1_rb, scale = (c1_rt/c1_rb));
    translate([0, 0, spline_offset]) small_spline(17);
    translate([0, 0, c2_offset]) rotate(a = 360 / 33 / 2) cylinder_extrude($fn = 33, h = 5, r = c2_rb, scale = (c2_rt/c2_rb));
    translate([0, 0, c3_offset]) rotate(a = 360 / 33 / 2) cylinder_extrude($fn = 33, h = 10, r = c3_rb, scale = (c3_rt/c3_rb));
}

module big_shaft() {
    spline_offset = 21; // used to be 20
    allowance = 0.1 - 0.1; // BEFORE PRINTING: set to 0
    
    // cylinder 1 (below outer spline)
    c1_dt = 24;
    c1_db = 24;
    c1_offset = 12;
    
    // cylinder 2 (below cylinder 1)
    c2_dt = 24;
    c2_db = 29; // used to be 30
    c2_offset = 3;
    
    // cylinder 3 (below cylinder 2)
    
    c3_dt = 29;
    c3_db = 29;
    c3_offset = 0;
    
    // radius calculation
    c1_rt = (c1_dt + allowance) / 2;
    c1_rb = (c1_db + allowance) / 2;
    c2_rt = (c2_dt + allowance) / 2;
    c2_rb = (c2_db + allowance) / 2;
    c3_rt = (c3_dt + allowance) / 2;
    c3_rb = (c3_db + allowance) / 2;
    
    translate([0, 0, spline_offset]) big_spline(11);
    translate([0, 0, c1_offset]) rotate(a = 360 / 63 / 2) cylinder_extrude($fn = 63, h = 9, r = c1_rb, scale = (c1_rt/c1_rb)); // h used to be 10
    translate([0, 0, c2_offset]) rotate(a = 360 / 63 / 2) cylinder_extrude($fn = 63, h = 9, r = c2_rb, scale = (c2_rt/c2_rb));
    translate([0, 0, c3_offset]) rotate(a = 360 / 63 / 2) cylinder_extrude($fn = 63, h = 3, r = c3_rb, scale = (c3_rt/c3_rb));
}

// Example usage
rotate([0, 0, 0])
difference() {
    big_shaft();
    small_shaft();
}
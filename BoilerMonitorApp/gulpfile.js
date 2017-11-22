/// <binding BeforeBuild='copy' />

var gulp = require("gulp"),
    fs = require("fs"),
    less = require("gulp-less");


gulp.task('copy', function () {
    gulp.src('./scripts/angular*.js')
        .pipe(gulp.dest('./www/scripts/'));

    gulp.src('./Views/*.html')
        .pipe(gulp.dest('./www/Views/'));
});

onmessage = function (event) {

};

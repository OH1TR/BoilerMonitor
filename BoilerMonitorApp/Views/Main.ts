
namespace Main {

    interface IScope extends ng.IScope {
        events: controller;
        Temp1: string;
        Temp2: string;
        Smoke: string;
    }

    interface IData {
        temp1: string;
        temp2: string;
        smoke: string;
    }

    export class controller {

        constructor(protected $scope: IScope, protected $http: ng.IHttpService) {
            var self = this;
            self.$scope.Temp1 = '-';
            self.$scope.Temp2 = '-';
            self.$scope.Smoke = '-';
            
            $http.get('http://boilermonitorserver.azurewebsites.net/api/values/latest').then((data: any): ng.IPromise<any> => {
                self.$scope.Temp1 = data.data.temp1;
                self.$scope.Temp2 = data.data.temp2;
                self.$scope.Smoke = data.data.smoke;
                return (data);
            });
            
        }

    }

    var app = angular.module("app");
    app.controller("Main", ["$scope", "$http", ($scope, $http) => new controller($scope, $http)]);
}
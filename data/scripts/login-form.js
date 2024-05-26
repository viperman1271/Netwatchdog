import { checkLoginAndRedirect } from './login-redirect.js'

document.getElementById('login-form').addEventListener('submit', function(event) {
    event.preventDefault();

    var username = document.getElementById('username').value;
    var password = document.getElementById('password').value;

    var xhr = new XMLHttpRequest();
    xhr.open('POST', '/api/login', true);
    xhr.setRequestHeader('Content-Type', 'application/json;charset=UTF-8');

    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4 && xhr.status === 200) {
            var response = JSON.parse(xhr.responseText);
            console.log("TESTING");
            // console.log(xhr.responseText);
            // console.log(response);

            // Handle success (e.g., save token, redirect, etc.)
            localStorage.setItem('authToken', response.token);
            checkLoginAndRedirect();
        } else if (xhr.readyState === 4) {
            console.error('Login failed');
        }
    };

    var data = JSON.stringify({
        username: username,
        password: password
    });

    xhr.send(data);
});
function checkLoginAndRedirect() {
    const token = localStorage.getItem('authToken');
    if (token) {
        fetch('/api/protected', {
            method: 'GET',
            headers: {
                'Authorization': 'Bearer ' + token
            }
        })
        .then(response => {
            if (response.ok) {
                window.location.href = '/dashboard.html';
            }
        })
    }
}

document.addEventListener('DOMContentLoaded', function() {
    checkLoginAndRedirect();
});
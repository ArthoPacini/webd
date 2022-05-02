async function get_resume_from_node()
{
    let request = await fetch(`${window.location.origin}/api/v1/resume`);
    let json = await request.json();
    return json;
}

async function main()
{
    let response = await get_resume_from_node();
    const container_cards_handle = document.getElementById('card-container');

    for(var i in response)
    {
        container_cards_handle.innerHTML += `<div class="card" id="${response[i]["domain"]}">
        <h2> <a href="${response[i]["domain"]}">${response[i]["domain"]}</a> </h2>
        <p>${response[i]["description"]}</p>
        <span>${response[i]["version"]}.0</span>

    </div> `;
    }
}

function search()
{
    let input_handle = document.getElementById('searchBar');
    let input = input_handle.value;

    

    let cards = document.getElementsByClassName('card');

    Array.prototype.forEach.call(cards, function(el) {
        // Do stuff here
        el.style.display = 'block';
    });

    if(input.length == 0)
        return;

  
    Array.prototype.forEach.call(cards, function(el) {
        // Do stuff here
        if(el.id.search(input) == -1)
            el.style.display = 'none';
    }); 


}



window.addEventListener('DOMContentLoaded', (event) => {
    main();
});